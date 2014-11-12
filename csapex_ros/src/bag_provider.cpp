/// HEADER
#include "bag_provider.h"

/// COMPONENT
#include <csapex_ros/ros_message_conversion.h>

/// PROJECT
#include <utils_param/parameter_factory.h>
#include <utils_param/range_parameter.h>

/// SYSTEM
#include <boost/assign.hpp>
#include <csapex/utility/register_apex_plugin.h>
#include <sensor_msgs/Image.h>

CSAPEX_REGISTER_CLASS(csapex::BagProvider, csapex::MessageProvider)

using namespace csapex;

BagProvider::BagProvider()
    : view_all_(NULL), initiated(false)
{
    std::vector<std::string> set;

    state.addParameter(param::ParameterFactory::declareBool("bag/play", true));
    state.addParameter(param::ParameterFactory::declareBool("bag/loop", true));
    state.addParameter(param::ParameterFactory::declareRange("bag/frame", 0, 1, 0, 1));

    param::Parameter::Ptr topic_param = param::ParameterFactory::declareParameterStringSet("topic",
                                                                                           param::ParameterDescription("topic to play <b>primarily</b>"), set, "");
    topic_param_ = boost::dynamic_pointer_cast<param::SetParameter>(topic_param);
    assert(topic_param_);
    state.addParameter(topic_param_);

}

BagProvider::~BagProvider()
{
}

void BagProvider::load(const std::string& file)
{
    file_ = file;
    bag.open(file_);

    RosMessageConversion& rmc = RosMessageConversion::instance();
    view_all_ = new rosbag::View(bag, rosbag::TypeQuery(rmc.getRegisteredRosTypes()));

    std::set<std::string> topics;
    for(rosbag::View::iterator it = view_all_->begin(); it != view_all_->end(); ++it) {
        rosbag::MessageInstance i = *it;
        topics.insert(i.getTopic());
    }


    if(!topics.empty()) {
        topics_.clear();
        topics_.assign(topics.begin(), topics.end());
        std::sort(topics_.begin(), topics_.end());
        topic_param_->setSet(topics_);
        topic_param_->set(topics_[0]);
    }

    view_it_ = view_all_->begin();
    for(std::size_t i = 0; i < topics_.size(); ++i) {
        view_it_map_[topics_[i]] = view_all_->end();
    }

    setSlotCount(topics.size());
}

void BagProvider::parameterChanged()
{
    setTopic();
}

void BagProvider::setTopic()
{
    if(!topic_param_->is<std::string>()) {
        return;
    }
    std::string main_topic = topic_param_->as<std::string>();

    if(main_topic == main_topic_) {
        return;
    }

    main_topic_ = main_topic;

    frames_ = 0;
    rosbag::View temp(bag, rosbag::TopicQuery(main_topic));
    for(rosbag::View::iterator it = temp.begin(); it != temp.end(); ++it) {
        frames_++;
    }
    frames_--;

    param::RangeParameter::Ptr frame = boost::dynamic_pointer_cast<param::RangeParameter>(state.getParameter("bag/frame"));
    frame->setMax(frames_);

    frame_ = 0;

    initiated = true;
}

std::vector<std::string> BagProvider::getExtensions() const
{
    return boost::assign::list_of<std::string> (".bag");
}

bool BagProvider::hasNext()
{
    if(!state.readParameter<bool>("bag/play")) {
        if(state.readParameter<int>("bag/frame") == frame_) {
            return false;
        } else {
            // frame was selected, ignore that play is false
        }
    }

    return initiated;
}

connection_types::Message::Ptr BagProvider::next(std::size_t slot)
{
    connection_types::Message::Ptr r;

    if(!initiated) {
        setTopic();
    }

    if(slot == 0) {
        // advance all iterators

        bool reset = false;
        int skip = 0;
        if(frame_ == frames_ || view_it_map_[main_topic_] == view_all_->end()) {
            // loop around?
            if(state.readParameter<bool>("bag/loop")) {
                reset = true;
            }

        } else if(state.readParameter<int>("bag/frame") != frame_) {
            // go to selected frame
            reset = true;
            skip = state.readParameter<int>("bag/frame");

        } else {
            // advance frame
            ++frame_;
        }

        if(reset) {
            frame_ = skip;
            view_it_ = view_all_->begin();
            for(std::size_t i = 0; i < topics_.size(); ++i) {
                view_it_map_[topics_[i]] = view_all_->end();
            }
        }

        bool done = false;
        for(; view_it_ != view_all_->end() && !done; ++view_it_) {
            rosbag::MessageInstance next = *view_it_;

            std::string topic = next.getTopic();

            // copy the iterator
            view_it_map_[topic] = rosbag::View::iterator(view_it_);

            if(topic == main_topic_) {
                if(skip > 0) {
                    --skip;
                } else {
                    done = true;
                }
            }
        }
    }

    std::string topic = topics_[slot];

    if(view_it_map_.find(topic) != view_it_map_.end() && view_it_map_[topic] != view_all_->end()) {
        rosbag::MessageInstance instance = *view_it_map_[topic];

        RosMessageConversion& rmc = RosMessageConversion::instance();
        r = rmc.instantiate(instance);
        setType(r->toType());
    }

    state["bag/frame"] = frame_;

    return r;
}

Memento::Ptr BagProvider::getState() const
{
    return Memento::Ptr();
}

void BagProvider::setParameterState(Memento::Ptr memento)
{

}
