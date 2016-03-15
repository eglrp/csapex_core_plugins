/// HEADER
#include "transform_publisher.h"

/// COMPONENT
#include <csapex_transform/transform_message.h>
#include <csapex_core_plugins/timestamp_message.h>

/// PROJECT
#include <csapex/msg/io.h>
#include <csapex/param/parameter_factory.h>
#include <csapex_ros/ros_handler.h>
#include <csapex/model/node_modifier.h>

/// SYSTEM
#include <csapex/utility/register_apex_plugin.h>

CSAPEX_REGISTER_CLASS(csapex::TransformPublisher, csapex::Node)

using namespace csapex;
using namespace csapex::connection_types;

TransformPublisher::TransformPublisher()
    : tfb_(nullptr)
{
}

void TransformPublisher::setupParameters(Parameterizable &parameters)
{
    parameters.addParameter(csapex::param::ParameterFactory::declareText("from", "/"));
    parameters.addParameter(csapex::param::ParameterFactory::declareText("to", "/"));
}

TransformPublisher::~TransformPublisher()
{
    delete tfb_;
}

void TransformPublisher::setupROS()
{
}

void TransformPublisher::processROS()
{
    if(!tfb_) {
        tfb_ = new tf::TransformBroadcaster;
    }

    ros::Time time;
    if(msg::hasMessage(input_time)) {
        TimestampMessage::ConstPtr time_msg = msg::getMessage<TimestampMessage>(input_time);
        auto nano = std::chrono::duration_cast<std::chrono::nanoseconds>(time_msg->value.time_since_epoch());
        time.fromNSec(nano.count());
    } else {
        time = ros::Time::now();
    }

    TransformMessage::ConstPtr trafo_msg = msg::getMessage<TransformMessage>(input_transform);

    tfb_->sendTransform(tf::StampedTransform(trafo_msg->value, time, readParameter<std::string>("from"), readParameter<std::string>("to")));
}


void TransformPublisher::setup(NodeModifier& node_modifier)
{
    input_transform = node_modifier.addInput<connection_types::TransformMessage>("T");
    input_time = node_modifier.addOptionalInput<connection_types::TimestampMessage>("time");
}
