/// HEADER
#include "statistical_outlier_removal.h"

/// PROJECT
#include <csapex/msg/io.h>
#include <csapex_point_cloud/msg/indices_message.h>
#include <csapex/param/parameter_factory.h>
#include <csapex/model/node_modifier.h>
#include <csapex/utility/register_apex_plugin.h>

/// SYSTEM
#include <csapex/utility/suppress_warnings_start.h>
    #include <pcl/point_types.h>
    #include <pcl/filters/statistical_outlier_removal.h>
#include <csapex/utility/suppress_warnings_end.h>

CSAPEX_REGISTER_CLASS(csapex::StatisticalOutlierRemoval, csapex::Node)

using namespace csapex;
using namespace csapex::connection_types;

StatisticalOutlierRemoval::StatisticalOutlierRemoval()
{
}

void StatisticalOutlierRemoval::setupParameters(Parameterizable &parameters)
{
    parameters.addParameter(csapex::param::ParameterFactory::declareRange("mean k", 1, 100, 2, 1));
    parameters.addParameter(csapex::param::ParameterFactory::declareBool ("keep organized", false));
    parameters.addParameter(csapex::param::ParameterFactory::declareBool ("negate", false));
    parameters.addParameter(csapex::param::ParameterFactory::declareRange("std dev threshold", 0.0, 10.0, 0.1, 0.1));
}

void StatisticalOutlierRemoval::setup(NodeModifier& node_modifier)
{
    input_cloud_    = node_modifier.addInput<PointCloudMessage>   ("PointCloud");
    input_indices_  = node_modifier.addOptionalInput<PointIndicesMessage> ("indices");
    output_cloud_   = node_modifier.addOutput<PointCloudMessage>  ("Pointcloud");
    output_indices_ = node_modifier.addOutput<PointIndicesMessage>("indices");
}

void StatisticalOutlierRemoval::process()
{
    PointCloudMessage::ConstPtr msg(msg::getMessage<PointCloudMessage>(input_cloud_));
    boost::apply_visitor (PointCloudMessage::Dispatch<StatisticalOutlierRemoval>(this, msg), msg->value);
}

template <class PointT>
void StatisticalOutlierRemoval::inputCloud(typename pcl::PointCloud<PointT>::ConstPtr cloud)
{
    bool indices_out = msg::isConnected(output_indices_);
    bool cloud_out   = msg::isConnected(output_cloud_);

    if(!indices_out && !cloud_out)
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
    if(msg::hasMessage(input_indices_)) {
        PointIndicesMessage::ConstPtr indices(msg::getMessage<PointIndicesMessage>(input_indices_));
        sor.setIndices(indices->value);
    }
    if(cloud_out) {
        typename pcl::PointCloud<PointT>::Ptr cloud_filtered(new pcl::PointCloud<PointT>);
        sor.filter(*cloud_filtered);
        PointCloudMessage::Ptr out(new PointCloudMessage(cloud->header.frame_id, cloud->header.stamp));
        out->value = cloud_filtered;
        msg::publish(output_cloud_, out);
    }
    if(indices_out) {
        PointIndicesMessage::Ptr indices_filtered(new PointIndicesMessage);
        indices_filtered->value->header = cloud->header;
        sor.filter(indices_filtered->value->indices);
        msg::publish(output_indices_, indices_filtered);
    }
}
