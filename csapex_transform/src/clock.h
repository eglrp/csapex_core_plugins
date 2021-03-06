#ifndef CLOCK_H
#define CLOCK_H

/// PROJECT
#include <csapex/model/node.h>

namespace csapex {

class Clock : public Node
{
private:
    enum Type {
        ZERO = 0,
        CURRENT = 1,
        CHRONO = 2
    };

public:    
    Clock();

    virtual void process() override;
    virtual void setup(csapex::NodeModifier& node_modifier) override;
    virtual void setupParameters(Parameterizable &parameters) override;


private:
    Output* output_;
};

}

#endif // CLOCK_H
