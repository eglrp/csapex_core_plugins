/// HEADER
#include "segmentation_renderer.h"

/// COMPONENT
#include <cslibs_laser_processing/segmentation/p2pline.h>

/// PROJECT
#include <csapex_ros/ros_message_conversion.h>
#include <csapex/msg/generic_vector_message.hpp>
#include <csapex/utility/register_apex_plugin.h>
#include <csapex/msg/io.h>
#include <csapex_opencv/cv_mat_message.h>
#include <csapex/param/parameter_factory.h>
#include <cslibs_laser_processing/data/segment.h>
#include <csapex/view/utility/color.hpp>
#include <csapex/model/node_modifier.h>
#include <cslibs_laser_processing/common/yaml-io.hpp>
#include <csapex_ros/yaml_io.hpp>
#include <csapex_scan_2d/binary_io.h>

/// SYSTEM
#include <visualization_msgs/MarkerArray.h>

using namespace csapex;
using namespace csapex::connection_types;
using namespace lib_laser_processing;

CSAPEX_REGISTER_CLASS(csapex::ScanSegmentation2DRenderer, csapex::Node)


ScanSegmentation2DRenderer::ScanSegmentation2DRenderer() :
    last_id_(0)
{
}

void ScanSegmentation2DRenderer::setupParameters(Parameterizable &parameters)
{
    parameters.addParameter(csapex::param::ParameterFactory::declareBool("publish marker", false));

    parameters.addParameter(csapex::param::ParameterFactory::declareBool("use random color", false));
    parameters.addParameter(csapex::param::ParameterFactory::declareBool("draw all points", false));

    parameters.addParameter(csapex::param::ParameterFactory::declareRange("width", 100, 2000, 1000, 1));
    parameters.addParameter(csapex::param::ParameterFactory::declareRange("height", 100, 2000, 1000, 1));

    parameters.addParameter(csapex::param::ParameterFactory::declareRange("scale", 0.1, 15.0, 1.0, 0.1));

    parameters.addParameter(csapex::param::ParameterFactory::declareColorParameter("color/segments", 0xFF, 0xCC, 0x00));
    parameters.addParameter(csapex::param::ParameterFactory::declareColorParameter("color/segments/classified", 0xFF, 0x00, 0x00));
    parameters.addParameter(csapex::param::ParameterFactory::declareColorParameter("color/bg", 0x00, 0x00, 0x00));
}

void ScanSegmentation2DRenderer::process()
{
    std::shared_ptr<std::vector<Segment> const> segments = msg::getMessage<GenericVectorMessage, Segment>(input_);
    if(readParameter<bool>("publish marker")) {
        publishMarkers(*segments);
    }

    if(msg::isConnected(output_)) {
        render(*segments);
    }
}

void ScanSegmentation2DRenderer::setup(NodeModifier& node_modifier)
{
    input_  = node_modifier.addInput<GenericVectorMessage, Segment>("Segments");
    output_ = node_modifier.addOutput<CvMatMessage>("Rendered");
    output_marker_ = node_modifier.addOutput<visualization_msgs::MarkerArray>("Marker");
}

void ScanSegmentation2DRenderer::render(const std::vector<Segment> &segments)
{
    CvMatMessage::Ptr output(new CvMatMessage(enc::bgr, "unknown", 0));

    int w = readParameter<int>("width");
    int h = readParameter<int>("height");

    const std::vector<int>& bg_color = readParameter<std::vector<int> >("color/bg");
    cv::Scalar bgColor(bg_color[2], bg_color[1], bg_color[0]);

    output->value = cv::Mat(h, w, CV_8UC3, bgColor);

    cv::Point2f origin(w/2, h/2);
    double scale = readParameter<double>("scale") * 10.0;

    bool random_color = readParameter<bool>("use random color");

    const std::vector<int>& color = readParameter<std::vector<int> >("color/segments");
    cv::Scalar defaultColor(color[2], color[1], color[0]);

    const std::vector<int>& color_classified = readParameter<std::vector<int> >("color/segments/classified");
    cv::Scalar classifiedColor(color_classified[2], color_classified[1], color_classified[0]);

    bool draw_all_points = readParameter<bool>("draw all points");

    int n = segments.size();
    for(int i = 0; i < n; ++i) {
        const Segment& segment = segments[i];

        cv::Scalar color;
        if(random_color) {
            double r = 0, g = 0, b = 0;
            color::fromCount(i, r,g,b);
            color = cv::Scalar(b,g,r);
        } else {
            if(segment.classification != 0) {
                color = classifiedColor;
            } else {
                color = defaultColor;
            }
        }

        if(segment.rays.empty()) {
            continue;
        }

        if(draw_all_points) {
            cv::Point2f last(segment.rays.front().posX(), segment.rays.front().posY());
            for(std::size_t i = 1, n = segment.rays.size(); i < n; ++i) {
                const Eigen::Vector2d pt(segment.rays[i].posX(), segment.rays[i].posY());
                cv::Point2f current(pt(0), pt(1));
                cv::line(output->value, origin + scale * last, origin + scale * current, color, 6, CV_AA);
                last = current;
            }

        } else {
            cv::Point2f from(segment.rays.front().posX(), segment.rays.front().posY());
            cv::Point2f to(segment.rays.back().posX(), segment.rays.back().posY());
            cv::line(output->value, origin + scale * from, origin + scale * to, color, 6, CV_AA);
        }
    }

    msg::publish(output_, output);
}

void ScanSegmentation2DRenderer::publishMarkers(const std::vector<Segment> &segments)
{
    visualization_msgs::MarkerArray::Ptr marker_array(new visualization_msgs::MarkerArray);
    visualization_msgs::Marker           marker;
    marker.id              = 1;
    marker.header.stamp    = ros::Time::now();
    marker.ns              = "scan_segmentation2D";
    marker.type            = visualization_msgs::Marker::LINE_LIST;
    marker.action          = visualization_msgs::Marker::ADD;
    marker.scale.x         = 0.1;
    marker.color.a = 1.0;
    int    h = 0.0;
    double r = 0, g = 0, b = 0;

    for(std::vector<Segment>::const_iterator it_seg = segments.begin() ; it_seg != segments.end() ; ++it_seg) {
        marker.points.clear();
        color::fromCount(h, r,g,b);
        marker.color.r = r / 255.0;
        marker.color.g = g / 255.0;
        marker.color.b = b / 255.0;

        marker.points.push_back(toGeometry(Eigen::Vector2d(it_seg->rays.front().posX(), it_seg->rays.front().posY())));
        for(std::vector<LaserBeam>::const_iterator it = it_seg->rays.begin() ; it != it_seg->rays.end(); ++it) {
            marker.points.push_back(toGeometry(Eigen::Vector2d(it->posX(), it->posY())));
            marker.points.push_back(marker.points.back());
        }
        marker.points.push_back(toGeometry(Eigen::Vector2d(it_seg->rays.back().posX(), it_seg->rays.back().posY())));
        marker.header.frame_id = it_seg->frame_id;
        marker_array->markers.push_back(marker);
        ++marker.id;
        ++h;
    }

    int last_id = last_id_;
    last_id_ = marker.id;

    for(int i = marker.id ; i < last_id ; ++i) {
        marker.id = i;
        marker.action = visualization_msgs::Marker::DELETE;
        marker_array->markers.push_back(marker);
    }

    msg::publish<visualization_msgs::MarkerArray>(output_marker_, marker_array);

}

geometry_msgs::Point ScanSegmentation2DRenderer::toGeometry(const Eigen::Vector2d &vec)
{
    geometry_msgs::Point p;
    p.x = vec.x();
    p.y = vec.y();
    return p;
}
