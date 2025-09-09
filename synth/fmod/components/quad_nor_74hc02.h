/**
 * @file quad_nor_74hc02.h
 * @brief 74HC02 Quad NOR gate - Component interface implementation
 */

#ifndef QUAD_NOR_74HC02_H
#define QUAD_NOR_74HC02_H

#include "../component_base.h"
#include <map>
#include <string>
#include <vector>

class QuadNOR_74HC02 : public Component {
public:
    enum Pin {
        GATE1_Y = 1, GATE1_A = 2, GATE1_B = 3,
        GATE2_Y = 4, GATE2_A = 5, GATE2_B = 6,
        GND = 7, GATE3_B = 8, GATE3_A = 9, GATE3_Y = 10,
        GATE4_B = 11, GATE4_A = 12, GATE4_Y = 13, VCC = 14
    };

    QuadNOR_74HC02();
    void setPin(int pin, LogicLevel level) override;
    LogicLevel getPin(int pin) const override;
    void setPower(bool on) override;
    bool isPowerOn() const override;
    double getPropagationDelay() const override;

    void setGateInputs(int gateNumber, LogicLevel inputA, LogicLevel inputB);
    LogicLevel getGateOutput(int gateNumber) const;

private:
    std::map<int, LogicLevel> pinStates;
    bool powerOn;
    static constexpr double PROPAGATION_DELAY_NS = 8.0;

    struct Gate {
        int inputA, inputB, output;
        std::string name;
    };
    std::vector<Gate> gates;

    bool isInputPin(int pin) const;
    void updateOutputs();
    LogicLevel norLogic(LogicLevel a, LogicLevel b) const;
};

#endif // QUAD_NOR_74HC02_H


