/// HEADER
#include <csapex_ml/features_message.h>

/// PROJECT
#include <csapex/utility/assert.h>
#include <csapex/utility/register_msg.h>

CSAPEX_REGISTER_MESSAGE(csapex::connection_types::FeaturesMessage)

using namespace csapex;
using namespace connection_types;

FeaturesMessage::FeaturesMessage()
    : Message("FeatureMessage", "/"), classification(0)
{}

ConnectionType::Ptr FeaturesMessage::clone() {
    Ptr new_msg(new FeaturesMessage);
    new_msg->value = value;
    new_msg->classification = classification;
    return new_msg;
}

ConnectionType::Ptr FeaturesMessage::toType() {
    Ptr new_msg(new FeaturesMessage);
    return new_msg;
}



/// YAML
namespace YAML {
Node convert<csapex::connection_types::FeaturesMessage>::encode(const csapex::connection_types::FeaturesMessage& rhs)
{
    Node node = convert<csapex::connection_types::Message>::encode(rhs);
    node["features"] = rhs.value;
    node["classification"] = rhs.classification;
    return node;
}

bool convert<csapex::connection_types::FeaturesMessage>::decode(const Node& node, csapex::connection_types::FeaturesMessage& rhs)
{
    if(!node.IsMap()) {
        return false;
    }
    convert<csapex::connection_types::Message>::decode(node, rhs);
    rhs.value = node["features"].as<std::vector<float> >();
    rhs.classification = node["classification"].as<int>();
    return true;
}
}
