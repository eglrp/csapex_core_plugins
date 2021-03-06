#ifndef GRADIENT_BOOSTED_TREES_H
#define GRADIENT_BOOSTED_TREES_H

/// COMPONENT
#include <csapex_ml/features_message.h>

/// PROJECT
#include <csapex/model/node.h>

/// SYSTEM
#include <opencv2/opencv.hpp>

namespace csapex
{
class CSAPEX_EXPORT_PLUGIN GradientBoostedTrees : public Node
{
public:
    GradientBoostedTrees() = default;

    virtual void setupParameters(Parameterizable &parameters) override;
    virtual void setup(NodeModifier &node_modifier) override;
    virtual void process() override;

private:
    std::shared_ptr<cv::GradientBoostingTrees> trees_;

    Input   *in_;
    Output  *out_;

    Slot    *reload_;

    void load();
    void classify(const connection_types::FeaturesMessage &input,
                  connection_types::FeaturesMessage &output);
};
}

#endif // GRADIENT_BOOSTED_TREES_H
