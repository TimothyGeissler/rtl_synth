/**
 * @file quad_xor_74hc86.h
 * @brief 74HC86 Quad XOR gate - Component interface implementation
 */

#ifndef QUAD_XOR_74HC86_H
#define QUAD_XOR_74HC86_H

#include "../component_base.h"
#include <map>
#include <string>
#include <vector>

class QuadXOR_74HC86 : public Component {
public:
    enum Pin {
        GATE1_A = 1, GATE1_B = 2, GATE1_Y = 3,
        GATE2_A = 4, GATE2_B = 5, GATE2_Y = 6,
        GND = 7, GATE3_Y = 8, GATE3_A = 9, GATE3_B = 10,
        GATE4_Y = 11, GATE4_A = 12, GATE4_B = 13, VCC = 14
    };

    QuadXOR_74HC86();
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
    LogicLevel xorLogic(LogicLevel a, LogicLevel b) const;
};

#endif // QUAD_XOR_74HC86_H


