/// HEADER
#include "to_feature.h"

/// PROJECT
#include <csapex/msg/io.h>
#include <csapex/msg/message.h>
#include <csapex/factory/message_factory.h>
#include <csapex/model/token_data.h>
#include <csapex/param/parameter_factory.h>
#include <csapex/param/path_parameter.h>
#include <csapex/utility/register_apex_plugin.h>
#include <csapex/model/node_modifier.h>

/// CONCRETE MESSAGES
#include <csapex_ml/features_message.h>
#include <csapex_opencv/cv_mat_message.h>
#include <csapex_vision_features/descriptor_message.h>
#include <csapex_vision_histograms/histogram_msg.h>

using namespace csapex;
using namespace connection_types;
using namespace csapex;

CSAPEX_REGISTER_CLASS(csapex::ToFeature, csapex::Node)

ToFeature::ToFeature()
{
}

void ToFeature::setup(NodeModifier& node_modifier)
{
    input_  = node_modifier.addInput<connection_types::AnyMessage>("anything");
    output_ = node_modifier.addOutput<GenericVectorMessage, FeaturesMessage>("features");
}

void ToFeature::setupParameters(Parameterizable& parameters)
{
    parameters.addParameter(csapex::param::ParameterFactory::declareRange("class id", 0, 255, 0, 1));
}

namespace {
typedef std::shared_ptr<std::vector<FeaturesMessage> > Features;

template<typename T>
inline void doProcessSingle(const T   &src,
                            std::shared_ptr< std::vector<FeaturesMessage> >  &dst,
                            const int label = 0)
{
    std::runtime_error("Unsupported message type!");
}

template<>
inline void doProcessSingle(const typename HistogramMessage::ConstPtr &src,
                            std::shared_ptr< std::vector<FeaturesMessage> > &dst,
                            const int label)
{
    for(std::vector<cv::Mat>::const_iterator
        it  = src->value.histograms.begin() ;
        it != src->value.histograms.end() ;
        ++it) {
        FeaturesMessage msg;
        msg.type = FeaturesMessage::Type::CLASSIFICATION;
        msg.classification = label;
        it->copyTo(msg.value);
        dst->push_back(msg);
    }
}

template<>
inline void doProcessSingle(const typename CvMatMessage::ConstPtr &src,
                            std::shared_ptr< std::vector<FeaturesMessage> > &dst,
                            const int label)
{
    FeaturesMessage features;
    cv::Mat tmp;
    src->value.copyTo(tmp);
    tmp.convertTo(tmp, CV_32F);
    tmp = tmp.reshape(1, 1);
    tmp.copyTo(features.value);

    features.classification = label;
    features.type = FeaturesMessage::Type::CLASSIFICATION;
    dst->push_back(features);
}

template<>
inline void doProcessSingle(const typename DescriptorMessage::ConstPtr &src,
                            std::shared_ptr< std::vector<FeaturesMessage> > &dst,
                            const int label)
{
    for(int i = 0 ; i < src->value.rows ; ++i) {
        FeaturesMessage features;
        features.type = FeaturesMessage::Type::CLASSIFICATION;
        src->value.row(i).copyTo(features.value);
        features.classification = label;
        dst->push_back(features);
    }
}

inline void processSingle(const TokenData::ConstPtr                    &src,
                          std::shared_ptr< std::vector<FeaturesMessage> > &dst,
                          const int                                         label)
{
    CvMatMessage::ConstPtr      cv =
            std::dynamic_pointer_cast<CvMatMessage const>(src);
    if(cv)
        doProcessSingle(cv, dst, label);

    DescriptorMessage::ConstPtr d  =
            std::dynamic_pointer_cast<DescriptorMessage const>(src);
    if(d)
        doProcessSingle(d, dst, label);

    HistogramMessage::ConstPtr  h  =
            std::dynamic_pointer_cast<HistogramMessage const>(src);
    if(h)
        doProcessSingle(h, dst, label);
}

inline void processVector(const connection_types::GenericVectorMessage::ConstPtr    &src,
                          std::shared_ptr< std::vector<FeaturesMessage> >  &dst,
                          const int label)
{
    for(std::size_t i = 0, total = src->nestedValueCount(); i < total; ++i) {
        processSingle(src->nestedValue(i), dst, label);
    }
}
}


void ToFeature::process()
{
    TokenData::ConstPtr msg = msg::getMessage<TokenData>(input_);
    std::shared_ptr< std::vector<FeaturesMessage> > out (new std::vector<FeaturesMessage>);


    int class_id = readParameter<int>("class id");

    connection_types::GenericVectorMessage::ConstPtr vector =
            std::dynamic_pointer_cast<connection_types::GenericVectorMessage const>(msg);
    if(vector) {
        processVector(vector, out, class_id);
    } else {
        processSingle(msg, out, class_id);
    }

    msg::publish<GenericVectorMessage, FeaturesMessage>(output_, out);
}

