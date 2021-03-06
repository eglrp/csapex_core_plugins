/// HEADER
#include <csapex_core_plugins/composite_message.h>

/// COMPONENT
#include <csapex/msg/token_traits.h>
#include <csapex/utility/register_msg.h>
#include <csapex/serialization/yaml.h>
#include <csapex/serialization/io/std_io.h>
#include <csapex/serialization/io/csapex_io.h>
#include <csapex/msg/any_message.h>

CSAPEX_REGISTER_MESSAGE(csapex::connection_types::CompositeMessage)

using namespace csapex;
using namespace connection_types;


CompositeMessage::CompositeMessage(const std::string& frame_id, Message::Stamp stamp)
    : Message ("MessageComposite", frame_id, stamp)
{
    type_ = csapex::makeEmpty<AnyMessage>();
}
CompositeMessage::CompositeMessage(TokenData::Ptr type, const std::string& frame_id, Message::Stamp stamp)
    : Message ("MessageComposite", frame_id, stamp)
{
    setDescriptiveName("Composite");
    type_ = type;
}

TokenData::Ptr CompositeMessage::getSubType() const
{
    return type_;
}

CompositeMessage::Ptr CompositeMessage::make(){
    Ptr new_msg(new CompositeMessage("/"));
    return new_msg;
}

bool CompositeMessage::canConnectTo(const TokenData *other_side) const
{
    const CompositeMessage* vec = dynamic_cast<const CompositeMessage*> (other_side);
    if(vec != 0 && type_->canConnectTo(vec->getSubType().get())) {
        return true;
    } else {
        return other_side->canConnectTo(this);
    }
}

bool CompositeMessage::acceptsConnectionFrom(const TokenData *other_side) const
{
    const CompositeMessage* vec = dynamic_cast<const CompositeMessage*> (other_side);
    if(vec != 0 && type_->acceptsConnectionFrom(vec->getSubType().get())) {
        return true;
    } else {
        return other_side->acceptsConnectionFrom(this);
    }
}

void CompositeMessage::serialize(SerializationBuffer &data, SemanticVersion& version) const
{
    Message::serialize(data, version);
    data << value;
}
void CompositeMessage::deserialize(const SerializationBuffer& data, const SemanticVersion& version)
{
    Message::deserialize(data, version);
    data >> value;
}

/// YAML
namespace YAML {
Node convert<csapex::connection_types::CompositeMessage>::encode(const csapex::connection_types::CompositeMessage& rhs)
{
    Node node = convert<csapex::connection_types::Message>::encode(rhs);
    node["values"] = rhs.value;
    return node;
}

bool convert<csapex::connection_types::CompositeMessage>::decode(const Node& node, csapex::connection_types::CompositeMessage& rhs)
{
    if(!node.IsMap()) {
        return false;
    }
    convert<csapex::connection_types::Message>::decode(node, rhs);
    rhs.value = node["values"].as<std::vector<TokenDataConstPtr>>();
    return true;
}
}

