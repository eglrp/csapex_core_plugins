/// HEADER
#include "threshold_noise_filter.h"

/// PROJECT
#include <csapex/utility/register_apex_plugin.h>
#include <csapex/msg/io.h>
#include <csapex/param/parameter_factory.h>
#include <csapex_opencv/cv_mat_message.h>
#include <csapex/model/node_modifier.h>
#include <cslibs_vision/utils/noise_filter.hpp>

using namespace csapex;
using namespace csapex::connection_types;
using namespace csapex;

CSAPEX_REGISTER_CLASS(csapex::ThresholdNoiseFilter, csapex::Node)

ThresholdNoiseFilter::ThresholdNoiseFilter()
{
}


void ThresholdNoiseFilter::process()
{
    CvMatMessage::ConstPtr input = msg::getMessage<CvMatMessage>(input_);
    CvMatMessage::ConstPtr threshold = msg::getMessage<CvMatMessage>(threshold_);
    CvMatMessage::Ptr output(new CvMatMessage(input->getEncoding(), input->frame_id, input->stamp_micro_seconds));

    if(!threshold->hasChannels(1, CV_8U)) {
        throw std::runtime_error("Threshold needs to be one channel grayscale!");
    }

    bool  interpolate = readParameter<bool>("interpolate");
    uchar thresh = readParameter<int>("threshold");
    int   type   = input->value.type();
    switch(type) {
    case CV_8UC1:
        if(interpolate)
            cslibs_vision::ThresholdNoiseFilter<uchar,uchar>::interpolate
                    (input->value, threshold->value, thresh, output->value);
        else
            cslibs_vision::ThresholdNoiseFilter<uchar,uchar>::filter
                    (input->value, threshold->value, thresh, output->value);
        break;
    case CV_8SC1:
        if(interpolate)
            cslibs_vision::ThresholdNoiseFilter<char,uchar>::interpolate
                    (input->value, threshold->value, thresh, output->value);
        else
            cslibs_vision::ThresholdNoiseFilter<char,uchar>::filter
                    (input->value, threshold->value, thresh, output->value);
        break;
    case CV_16UC1:
        if(interpolate)
            cslibs_vision::ThresholdNoiseFilter<ushort,uchar>::interpolate
                    (input->value, threshold->value, thresh, output->value);
        else
            cslibs_vision::ThresholdNoiseFilter<ushort,uchar>::filter
                    (input->value, threshold->value, thresh, output->value);
        break;
    case CV_16SC1:
        if(interpolate)
            cslibs_vision::ThresholdNoiseFilter<short,uchar>::interpolate
                    (input->value, threshold->value, thresh, output->value);
        else
            cslibs_vision::ThresholdNoiseFilter<short,uchar>::filter
                    (input->value, threshold->value, thresh, output->value);
        break;
    case CV_32SC1:
        if(interpolate)
            cslibs_vision::ThresholdNoiseFilter<int,uchar>::interpolate
                    (input->value, threshold->value, thresh, output->value);
        else
            cslibs_vision::ThresholdNoiseFilter<int,uchar>::filter
                    (input->value, threshold->value, thresh, output->value);
        break;
    case CV_32FC1:
        if(interpolate)
            cslibs_vision::ThresholdNoiseFilter<float,uchar>::interpolate
                    (input->value, threshold->value, thresh, output->value);
        else
            cslibs_vision::ThresholdNoiseFilter<float,uchar>::filter
                    (input->value, threshold->value, thresh, output->value);
        break;
    case CV_64FC1:
        if(interpolate)
            cslibs_vision::ThresholdNoiseFilter<double,uchar>::interpolate
                    (input->value, threshold->value, thresh, output->value);
        else
            cslibs_vision::ThresholdNoiseFilter<double,uchar>::filter
                    (input->value, threshold->value, thresh, output->value);
        break;
    }

    msg::publish(output_, output);
}

void ThresholdNoiseFilter::setup(NodeModifier& node_modifier)
{
    input_      = node_modifier.addInput<CvMatMessage>("unfiltered");
    threshold_  = node_modifier.addInput<CvMatMessage>("weights");
    output_     = node_modifier.addOutput<CvMatMessage>("filtered");
}

void ThresholdNoiseFilter::setupParameters(Parameterizable& parameters)
{
    parameters.addParameter(csapex::param::ParameterFactory::declareRange("threshold", 0, 255, 255, 1));
    parameters.addParameter(csapex::param::ParameterFactory::declareBool("interpolate", false));
}
