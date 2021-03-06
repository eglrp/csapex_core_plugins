/// HEADER
#include "pointcloud_to_intensityimage.h"

/// PROJECT
#include <csapex_opencv/cv_mat_message.h>
#include <csapex/msg/io.h>
#include <csapex_core_plugins/timestamp_message.h>
#include <csapex/param/parameter_factory.h>
#include <csapex/model/node_modifier.h>
#include <csapex/utility/register_apex_plugin.h>

CSAPEX_REGISTER_CLASS(csapex::PointCloudToIntensityImage, csapex::Node)

using namespace csapex;
using namespace csapex::connection_types;

PointCloudToIntensityImage::PointCloudToIntensityImage()
{
}

void PointCloudToIntensityImage::setupParameters(Parameterizable& parameters)
{
    parameters.addParameter(
            param::ParameterFactory::declareBool("skip_invalid", true),
            skip_invalid_);
}

void PointCloudToIntensityImage::setup(NodeModifier& node_modifier)
{
    input_ = node_modifier.addInput<PointCloudMessage>("PointCloud");

    output_ = node_modifier.addOutput<CvMatMessage>("Intensity Image");
}

void PointCloudToIntensityImage::process()
{
    PointCloudMessage::ConstPtr msg(msg::getMessage<PointCloudMessage>(input_));

    boost::apply_visitor (PointCloudMessage::Dispatch<PointCloudToIntensityImage>(this, msg), msg->value);
}

namespace impl {

template <class PointT>
struct Impl
{
    static void inputCloud(PointCloudToIntensityImage* instance, typename pcl::PointCloud<PointT>::ConstPtr cloud)
    {
        throw std::runtime_error(std::string("point type '") + type2name(typeid(PointT)) + "' not supported");
    }
};

template <>
struct Impl<pcl::PointXYZI>
{
    static void inputCloud(PointCloudToIntensityImage* instance, typename pcl::PointCloud<pcl::PointXYZI>::ConstPtr cloud)
    {
        instance->inputCloudImpl(cloud);
    }
};
}

template <class PointT>
void PointCloudToIntensityImage::inputCloud(typename pcl::PointCloud<PointT>::ConstPtr cloud)
{
    impl::Impl<PointT>::inputCloud(this, cloud);
}

void PointCloudToIntensityImage::inputCloudImpl(typename pcl::PointCloud<pcl::PointXYZI>::ConstPtr cloud)
{
    unsigned n = cloud->points.size();

    int cols = cloud->width;
    int rows = n / cols;

    CvMatMessage::Ptr output(new CvMatMessage(enc::mono, cloud->header.frame_id, cloud->header.stamp));
    output->value = cv::Mat(rows,cols, CV_16U, cv::Scalar::all(0));

    typename pcl::PointCloud<pcl::PointXYZI>::const_iterator pt = cloud->points.begin();
    ushort* data = (ushort*) output->value.data;

    for(unsigned idx = 0; idx < n; ++idx) {
        const pcl::PointXYZI& p = *pt;
        double dist = std::sqrt(p.x*p.x + p.y*p.y + p.z*p.z);
        if (!skip_invalid_ || dist == dist) {
            *data = p.intensity;
        }

        ++data;
        ++pt;
    }

    msg::publish(output_, output);
}
