#ifndef HOUGH_LINESP_H
#define HOUGH_LINESP_H

/// COMPONENT
#include "detection.h"

namespace csapex {
class HoughLinesP : public CornerDetection
{
public:
    HoughLinesP();

    virtual void allConnectorsArrived();
    virtual void setup();

protected:
    void update();

    double rho_;
    double theta_;
    int    threshold_;
    double min_line_length_;
    double max_line_gap_;
};
}
#endif // HOUGH_LINESP_H
