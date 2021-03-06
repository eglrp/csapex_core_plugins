
/// PROJECT
#include <csapex/model/node.h>
#include <csapex/msg/io.h>
#include <csapex/param/parameter_factory.h>
#include <csapex/model/node_modifier.h>
#include <csapex/utility/register_apex_plugin.h>
#include <csapex/msg/any_message.h>
#include <csapex/model/node_handle.h>

using namespace csapex;
using namespace csapex::connection_types;

namespace csapex
{
namespace state
{

class ActivityAbsorber : public Node
{
public:
    ActivityAbsorber()
    {
    }

    void setup(csapex::NodeModifier& modifier) override
    {
        in_ = modifier.addInput<AnyMessage>("Input");
        out_ = modifier.addOutput<AnyMessage>("Output");
    }

    void setupParameters(csapex::Parameterizable& params) override
    {

    }

    void activation() override
    {
        node_handle_->setActive(false);
    }

    void deactivation() override
    {
    }

    void process() override
    {
        auto message = msg::getMessage(in_);
        apex_assert(message);

        msg::publish(out_, message);
    }

private:
    Input* in_;
    Output* out_;
};

}
}

CSAPEX_REGISTER_CLASS(csapex::state::ActivityAbsorber, csapex::Node)

