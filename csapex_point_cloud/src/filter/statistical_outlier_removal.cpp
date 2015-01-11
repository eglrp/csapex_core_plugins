/// HEADER
#include "statistical_outlier_removal.h"

/// PROJECT
#include <csapex/msg/input.h>
#include <csapex/msg/output.h>
#include <csapex_point_cloud/indeces_message.h>
#include <utils_param/parameter_factory.h>
#include <csapex/model/node_modifier.h>
#include <csapex/utility/register_apex_plugin.h>

/// SYSTEM
#include <pcl/point_types.h>
#include <pcl/filters/statistical_outlier_removal.h>
#include <pcl/filters/approximate_voxel_grid.h>

CSAPEX_REGISTER_CLASS(csapex::StatisticalOutlierRemoval, csapex::Node)

using namespace csapex;
using namespace csapex::connection_types;

StatisticalOutlierRemoval::StatisticalOutlierRemoval()
{
    addParameter(param::ParameterFactory::declareRange("mean k", 1, 100, 2, 1));
    addParameter(param::ParameterFactory::declareBool ("keep organized", false));
    addParameter(param::ParameterFactory::declareBool ("negate", false));
    addParameter(param::ParameterFactory::declareRange("std dev threshold", 0.0, 10.0, 0.1, 0.1));
}

void StatisticalOutlierRemoval::setup()
{
    input_cloud_    = modifier_->addInput<PointCloudMessage>   ("PointCloud");
    input_indeces_  = modifier_->addOptionalInput<PointIndecesMessage> ("Indeces");
    output_cloud_   = modifier_->addOutput<PointCloudMessage>  ("Pointcloud");
    output_indeces_ = modifier_->addOutput<PointIndecesMessage>("Indeces");
}

void StatisticalOutlierRemoval::process()
{
    PointCloudMessage::ConstPtr msg(input_cloud_->getMessage<PointCloudMessage>());
    boost::apply_visitor (PointCloudMessage::Dispatch<StatisticalOutlierRemoval>(this, msg), msg->value);
}

template <class PointT>
void StatisticalOutlierRemoval::inputCloud(typename pcl::PointCloud<PointT>::ConstPtr cloud)
{
    bool indeces_out = output_indeces_->isConnected();
    bool cloud_out   = output_cloud_->isConnected();

    if(!indeces_out && !cloud_out)
        return;

    int    mean_k             = readParameter<int>("mean k");
    bool   keep_organized     = readParameter<bool>("keep organized");
    bool   negative           = readParameter<bool>("negate");
    double std_dev_mul_thresh = readParameter<double>("std dev threshold");

    pcl::StatisticalOutlierRemoval<PointT> sor;
    sor.setInputCloud(cloud);
    sor.setKeepOrganized(keep_organized);
    sor.setMeanK(mean_k);
    sor.setNegative(negative);
    sor.setStddevMulThresh(std_dev_mul_thresh);
    if(input_indeces_->hasMessage()) {
        PointIndecesMessage::ConstPtr indeces(input_indeces_->getMessage<PointIndecesMessage>());
        sor.setIndices(indeces->value);
    }
    if(cloud_out) {
        typename pcl::PointCloud<PointT>::Ptr cloud_filtered(new pcl::PointCloud<PointT>);
        sor.filter(*cloud_filtered);
        PointCloudMessage::Ptr out(new PointCloudMessage(cloud->header.frame_id, cloud->header.stamp));
        out->value = cloud_filtered;
        output_cloud_->publish(out);
    }
    if(indeces_out) {
        PointIndecesMessage::Ptr indeces_filtered(new PointIndecesMessage);
        indeces_filtered->value->header = cloud->header;
        sor.filter(indeces_filtered->value->indices);
        output_indeces_->publish(indeces_filtered);
    }
}
