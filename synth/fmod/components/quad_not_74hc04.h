#ifndef QUAD_NOT_74HC04_H
#define QUAD_NOT_74HC04_H

#include "../component_base.h"
#include <map>
#include <string>
#include <vector>

class HexInverter_74HC04 : public Component {
public:
    enum Pin {
        GATE1_A = 1, GATE1_Y = 2,
        GATE2_A = 3, GATE2_Y = 4,
        GATE3_A = 5, GATE3_Y = 6,
        GND = 7,
        GATE4_A = 9, GATE4_Y = 8,
        GATE5_A = 11, GATE5_Y = 10,
        GATE6_A = 13, GATE6_Y = 12,
        VCC = 14
    };

    HexInverter_74HC04();

    void setPin(int pin, LogicLevel level) override;
    LogicLevel getPin(int pin) const override;
    void setPower(bool on) override;
    bool isPowerOn() const override;
    double getPropagationDelay() const override;

    void setGateInput(int gateNumber, LogicLevel inputA);
    LogicLevel getGateOutput(int gateNumber) const;

private:
    std::map<int, LogicLevel> pinStates;
    bool powerOn;
    static constexpr double PROPAGATION_DELAY_NS = 8.0;

    struct Gate { int inputA, output; };
    std::vector<Gate> gates;

    bool isInputPin(int pin) const;
    void updateOutputs();
    LogicLevel notLogic(LogicLevel a) const;
};

#endif // QUAD_NOT_74HC04_H
