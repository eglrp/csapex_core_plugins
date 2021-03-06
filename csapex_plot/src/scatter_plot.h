#ifndef SCATTER_PLOT_H
#define SCATTER_PLOT_H

/// PROJECT
#include "plot.h"
#include <csapex/model/variadic_io.h>

/// SYSTEM
#include <chrono>
#include <QColor>
#include <qwt_plot_curve.h>
#include <qwt_plot_scaleitem.h>
#include <qwt_scale_map.h>
#include <deque>

namespace csapex
{

class ScatterPlot : public Plot, public csapex::VariadicInputs
{
public:
    virtual void setup(csapex::NodeModifier& node_modifier) override;
    virtual void setupParameters(Parameterizable& parameters) override;
    virtual void process() override;

    const double* getXData() const;
    const double* getYData() const;
    const double* getVarData(std::size_t idx) const;
    std::size_t getCount() const;
//    std::size_t getNumberOfPlots() const;


    double getPointSize() const;

private:
    void reset();


private:
    Input* in_x_;
    Input* in_y_;
    std::vector<double> x;
    std::vector<double> y;
    std::vector<std::vector<double>> var_y_;

    double min_x_;
    double min_y_;
    double max_x_;
    double max_y_;
};

}

#endif // SCATTER_PLOT_H
