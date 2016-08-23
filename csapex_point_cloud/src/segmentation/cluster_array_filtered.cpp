#include "cluster_array_filtered.h"

/// PROJECT
#include <csapex/msg/io.h>
#include <csapex/msg/generic_vector_message.hpp>
#include <csapex/param/parameter_factory.h>
#include <csapex/utility/register_apex_plugin.h>
#include <csapex/model/node_modifier.h>
#include <csapex_point_cloud/indeces_message.h>
#include <csapex/msg/generic_value_message.hpp>
#include <csapex/profiling/timer.h>
#include <csapex/profiling/interlude.hpp>
#include "cluster_utils.hpp"

#include <cslibs_kdtree/array_clustering.hpp>
#include <cslibs_kdtree/fill.hpp>

CSAPEX_REGISTER_CLASS(csapex::ClusterPointCloudArrayFiltered, csapex::Node)

using namespace csapex;
using namespace csapex::connection_types;

namespace {
using DataIndex           = std::array<int, 3>;

struct Entry {
    int                      cluster;
    DataIndex                index;
    std::vector<int>         indices;
    math::Distribution<3>    distribution;

    Entry() :
        cluster(-1)
    {
    }
};

using ArrayType            = kdtree::Array<Entry*, 3>;
using ArrayIndex           = ArrayType::Index;
using AO                  = kdtree::ArrayOperations<3, int, int>;
using AOA                 = kdtree::ArrayOperations<3, int, std::size_t>;

struct Indexation {
    using BinType = std::array<double, 3>;

    BinType bin_sizes;

    Indexation(const BinType &_bin_sizes) :
        bin_sizes(_bin_sizes)
    {
    }

    inline static ArrayType::Size size(const DataIndex &min_index,
                                       const DataIndex &max_index)
    {
        ArrayType::Size size;
        for(std::size_t i = 0 ; i < 3 ; ++i) {
            size[i] = (max_index[i] - min_index[i]) + 1;
        }
        return size;
    }

    template<typename PointT>
    inline constexpr bool is_valid(const PointT& point) const
    {
        bool nan = std::isnan(point.x) || std::isnan(point.y) || std::isnan(point.z);
        bool inf = std::isinf(point.x) || std::isinf(point.y) || std::isinf(point.z);
        return !(nan || inf);
    }

    template<typename PointT>
    inline constexpr DataIndex create(const PointT& point) const
    {
        return { static_cast<int>(std::floor(point.x / bin_sizes[0])),
                 static_cast<int>(std::floor(point.y / bin_sizes[1])),
                 static_cast<int>(std::floor(point.z / bin_sizes[2]))};
    }
};

class ArrayClustering
{
public:
    typedef kdtree::detail::fill<DataIndex, 3>   MaskFiller;
    typedef typename MaskFiller::Type            MaskType;
    typedef ClusterPointCloudArrayFiltered::ClusterParams ClusterParams;
    typedef Validator<ClusterParams>             ValidatorType;

    ArrayClustering(std::vector<Entry*>            &_entries,
                   const ClusterParams             &_params,
                   std::vector<pcl::PointIndices>  &_indices,
                   std::vector<pcl::PointIndices>  &_indices_rejected,
                   ArrayType                       &_array,
                   DataIndex                       &_min_index,
                   DataIndex                       &_max_index) :
        cluster_count(0),
        entries(_entries),
        indices(_indices),
        indices_rejected(_indices_rejected),
        array(_array),
        min_index(_min_index),
        max_index(_max_index),
        validator(_params, buffer_indices, buffer_distribution)
    {
        MaskFiller::assign(offsets);
    }

    inline void cluster()
    {
        for(Entry *entry : entries)
        {
            if(entry->cluster > -1)
                continue;

            ValidatorType::Result validation = validator.validate();
            if (validation == ValidatorType::Result::ACCEPTED)
                indices.emplace_back(std::move(buffer_indices));
            else
            {
                if (validation != ValidatorType::Result::TOO_SMALL)
                    indices_rejected.emplace_back(std::move(buffer_indices));
                else
                    buffer_indices.indices.clear();
            }

            buffer_distribution.reset();

            entry->cluster = cluster_count;
            ++cluster_count;
            clusterEntry(entry);
        }

        ValidatorType::Result validation = validator.validate();
        if (validation == ValidatorType::Result::ACCEPTED)
            indices.emplace_back(std::move(buffer_indices));
        else if (validation != ValidatorType::Result::TOO_SMALL)
            indices_rejected.emplace_back(std::move(buffer_indices));
    }

private:
    MaskType offsets;
    int      cluster_count;

    std::vector<Entry*>            &entries;
    std::vector<pcl::PointIndices> &indices;
    std::vector<pcl::PointIndices> &indices_rejected;
    ArrayType                      &array;
    DataIndex                      min_index;
    DataIndex                      max_index;

    ValidatorType                  validator;
    pcl::PointIndices              buffer_indices;
    math::Distribution<3>          buffer_distribution;

    inline void clusterEntry(Entry *entry)
    {
        ArrayIndex array_index;
        DataIndex index;
        for(DataIndex &offset : offsets) {
            if(AO::is_zero(offset))
                continue;

            AO::add(entry->index, offset, index);

            bool out_of_bounds = false;
            for(std::size_t j = 0 ; j < 3 ; ++j) {
                out_of_bounds |= index[j] < min_index[j];
                out_of_bounds |= index[j] > max_index[j];
                array_index[j]  = index[j] - min_index[j];
            }

            if(out_of_bounds)
                continue;

            Entry *neighbour = array.at(array_index);
            if(!neighbour)
                continue;
            if(neighbour->cluster > -1)
                continue;
            assert(neighbour->cluster == -1);

            if (validator.params.cluster_distance_and_weights[0] != 0.0)
            {
                using MeanType = math::Distribution<3>::PointType;
                MeanType diff = entry->distribution.getMean() - neighbour->distribution.getMean();
                diff(0) *= validator.params.cluster_distance_and_weights[1];
                diff(1) *= validator.params.cluster_distance_and_weights[2];
                diff(2) *= validator.params.cluster_distance_and_weights[3];
                auto dist = diff.dot(diff);
                if (dist > validator.params.cluster_distance_and_weights[0])
                    continue;
            }

            const int cluster = entry->cluster;
            neighbour->cluster = cluster;

            buffer_distribution += neighbour->distribution;
            buffer_indices.indices.insert(buffer_indices.indices.end(),
                                          neighbour->indices.begin(),
                                          neighbour->indices.end());

            clusterEntry(neighbour);
        }
    }
};


}

void ClusterPointCloudArrayFiltered::setup(NodeModifier &node_modifier)
{
    in_cloud_ = node_modifier.addInput<PointCloudMessage>("PointCloud");
    in_indices_ = node_modifier.addOptionalInput<PointIndecesMessage>("Indices");

    out_ = node_modifier.addOutput<GenericVectorMessage, pcl::PointIndices >("Clusters");
    out_rejected_ = node_modifier.addOutput<GenericVectorMessage, pcl::PointIndices >("Rejected Clusters");
}

void ClusterPointCloudArrayFiltered::setupParameters(Parameterizable &parameters)
{
    parameters.addParameter(param::ParameterFactory::declareRange("bin/size_x", 0.01, 8.0, 0.1, 0.01),
                            cluster_params_.bin_sizes[0]);
    parameters.addParameter(param::ParameterFactory::declareRange("bin/size_y", 0.01, 8.0, 0.1, 0.01),
                            cluster_params_.bin_sizes[1]);
    parameters.addParameter(param::ParameterFactory::declareRange("bin/size_z", 0.01, 8.0, 0.1, 0.01),
                            cluster_params_.bin_sizes[2]);
    parameters.addParameter(param::ParameterFactory::declareRange("cluster/min_size", 1, 1000000, 0, 1),
                            cluster_params_.cluster_sizes[0]);
    parameters.addParameter(param::ParameterFactory::declareRange("cluster/max_size", 1, 1000000, 1000000, 1),
                            cluster_params_.cluster_sizes[1]);
    parameters.addParameter(param::ParameterFactory::declareRange("cluster/max_distance", 0.00, 3.0, 0.0, 0.01),
                            cluster_params_.cluster_distance_and_weights[0]);
    parameters.addParameter(param::ParameterFactory::declareRange("cluster/distance_weights/x", 0.0, 1.0, 1.0, 0.01),
                            cluster_params_.cluster_distance_and_weights[1]);
    parameters.addParameter(param::ParameterFactory::declareRange("cluster/distance_weights/y", 0.0, 1.0, 1.0, 0.01),
                            cluster_params_.cluster_distance_and_weights[2]);
    parameters.addParameter(param::ParameterFactory::declareRange("cluster/distance_weights/z", 0.0, 1.0, 1.0, 0.01),
                            cluster_params_.cluster_distance_and_weights[3]);
    parameters.addParameter(param::ParameterFactory::declareInterval("cluster/std_dev/x", 0.0, 3.0, 0.0, 0.0, 0.01),
                            cluster_params_.cluster_std_devs[0]);
    parameters.addParameter(param::ParameterFactory::declareInterval("cluster/std_dev/y", 0.0, 3.0, 0.0, 0.0, 0.01),
                            cluster_params_.cluster_std_devs[1]);
    parameters.addParameter(param::ParameterFactory::declareInterval("cluster/std_dev/z", 0.0, 3.0, 0.0, 0.0, 0.01),
                            cluster_params_.cluster_std_devs[2]);

    std::map<std::string, int> covariance_threshold_types = {{"DEFAULT", ClusterParams::DEFAULT},
                                                             {"PCA2D", ClusterParams::PCA2D},
                                                             {"PCA3D", ClusterParams::PCA3D}};
    parameters.addParameter(param::ParameterFactory::declareParameterSet("cluster/std_dev_thresh_type",
                                                                         covariance_threshold_types,
                                                                         (int) ClusterParams::DEFAULT),
                            reinterpret_cast<int&>(cluster_params_.cluster_cov_thresh_type));
}

void ClusterPointCloudArrayFiltered::process()
{
    PointCloudMessage::ConstPtr msg(msg::getMessage<PointCloudMessage>(in_cloud_));
    boost::apply_visitor (PointCloudMessage::Dispatch<ClusterPointCloudArrayFiltered>(this, msg), msg->value);
}

template <class PointT>
void ClusterPointCloudArrayFiltered::inputCloud(typename pcl::PointCloud<PointT>::ConstPtr cloud)
{
    if (cloud->empty())
        return;

    pcl::PointIndicesPtr indices;
    if(msg::isConnected(in_indices_))
    {
        auto indices_msg = msg::getMessage<PointIndecesMessage>(in_indices_);
        indices = indices_msg->value;
    }

    std::shared_ptr<std::vector<pcl::PointIndices>> out_cluster_indices(new std::vector<pcl::PointIndices>);
    std::shared_ptr<std::vector<pcl::PointIndices>> out_rejected_cluster_indices(new std::vector<pcl::PointIndices>);
    DataIndex  min_index = AO::max();
    DataIndex  max_index = AO::min();
    Indexation indexation(cluster_params_.bin_sizes);

    std::vector<Entry>  entries;
    {
        /// Preparation of indices
        if(indices) {
            for(const int i : indices->indices) {
                const PointT &pt = cloud->at(i);
                if(indexation.is_valid(pt)) {
                    Entry entry;
                    entry.index = indexation.create(pt);
                    entry.indices.push_back(i);
                    entry.distribution.add({pt.x, pt.y, pt.z});
                    AO::cwise_min(entry.index, min_index);
                    AO::cwise_max(entry.index, max_index);
                    entries.emplace_back(entry);
                }
            }
        } else {
            const std::size_t cloud_size = cloud->size();
            for(std::size_t i = 0; i < cloud_size ; ++i) {
                const PointT &pt = cloud->at(i);
                if(indexation.is_valid(pt)) {
                    Entry entry;
                    entry.index = indexation.create(pt);
                    entry.indices.push_back(i);
                    entry.distribution.add({pt.x, pt.y, pt.z});
                    AO::cwise_min(entry.index, min_index);
                    AO::cwise_max(entry.index, max_index);
                    entries.push_back(entry);
                }
            }
        }
    }
    std::vector<Entry*> referenced;
    ArrayType::Size size = Indexation::size(min_index, max_index);
    ArrayType array(size);
    {
        /// Setup array adressing
        ArrayType::Index index;
        for(Entry &e : entries) {
            index = AOA::sub(e.index, min_index);
            Entry *& array_entry = array.at(index);
            if(!array_entry) {
                /// put into array
                array_entry = &e;
                referenced.emplace_back(array_entry);
            } else {
                /// fuse with existing
                std::vector<int> &array_entry_indices = array_entry->indices;
                array_entry_indices.insert(array_entry_indices.end(),
                                           e.indices.begin(),
                                           e.indices.end());
                array_entry->distribution += e.distribution;
            }
        }
    }
    {
        /// Clustering stage
        ArrayClustering clustering(referenced, cluster_params_, *out_cluster_indices, *out_rejected_cluster_indices, array, min_index, max_index);
        clustering.cluster();
    }
    msg::publish<GenericVectorMessage, pcl::PointIndices >(out_, out_cluster_indices);
    msg::publish<GenericVectorMessage, pcl::PointIndices >(out_rejected_, out_rejected_cluster_indices);
}
