#ifndef PLANE_SEGMENTATION_H
#define PLANE_SEGMENTATION_H

/// PROJECT
#include <csapex/model/node.h>

/// SYSTEM
#include <pcl/point_cloud.h>

namespace csapex {
class PlaneSegmentation : public csapex::Node
{
public:
    PlaneSegmentation() = default;

    virtual void setup(csapex::NodeModifier &node_modifier) override;
    virtual void setupParameters(Parameterizable &parameters) override;
    virtual void process() override;

    template<class PointT>
    void inputCloud(typename pcl::PointCloud<PointT>::ConstPtr cloud);

protected:
    Input  *input_cloud_;

    Output *ouput_normals_;

};
}

#endif // PLANE_SEGMENTATION_H
