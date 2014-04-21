/// HEADER
#include "foreach_adapter.h"

/// PROJECT
#include <csapex/model/connector_in.h>
#include <csapex/utility/register_node_adapter.h>
#include <csapex/view/port.h>
#include <csapex/model/connector_out.h>
#include <csapex/model/connector_in.h>
#include <csapex/view/widget_controller.h>

using namespace csapex;

CSAPEX_REGISTER_NODE_ADAPTER(ForeachAdapter, csapex::Foreach)

ForeachAdapter::ForeachAdapter(Foreach *node, WidgetController* widget_ctrl)
    : NodeAdapter(node, widget_ctrl), wrapped_(node)
{
}

void ForeachAdapter::setupUi(QBoxLayout* layout)
{
    QHBoxLayout* sub = new QHBoxLayout;

    widget_ctrl_->insertPort(sub, new Port(wrapped_->getCommandDispatcher(), wrapped_->out_sub));
    widget_ctrl_->insertPort(sub, new Port(wrapped_->getCommandDispatcher(), wrapped_->in_sub));

    layout->addLayout(sub);
}
