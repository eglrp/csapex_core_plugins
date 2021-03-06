#ifndef PASSTHROUGH_H_
#define PASSTHROUGH_H_

/// PROJECT
#include <csapex/model/node.h>
#include <csapex_point_cloud/msg/point_cloud_message.h>

namespace csapex {

class PassThroughByIndices : public Node
{
public:
    PassThroughByIndices();

    virtual void setup(csapex::NodeModifier& node_modifier) override;
    virtual void setupParameters(Parameterizable &parameters) override;
    virtual void process() override;

    template <class PointT>
    void inputCloud(typename pcl::PointCloud<PointT>::ConstPtr cloud);

private:
    Input  *input_cloud_;
    Input  *input_indices_;
    Output *output_pos_;
    Output *output_neg_;
};

}

#endif // PASSTHROUGH_H_
