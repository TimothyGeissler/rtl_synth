/**
 * @file dual_dff_74hc74.h
 * @brief 74HC74 Dual D-type flip-flop (positive edge) - simplified functional model
 */

#ifndef DUAL_DFF_74HC74_H
#define DUAL_DFF_74HC74_H

#include "../component_base.h"
#include <map>
#include <vector>

class DualDFF_74HC74 : public Component {
public:
    enum Pin {
        CLR1_N = 1, D1 = 2, CLK1 = 3, PRE1_N = 4, Q1 = 5, Q1_N = 6,
        GND = 7,
        Q2_N = 8, Q2 = 9, PRE2_N = 10, CLK2 = 11, D2 = 12, CLR2_N = 13,
        VCC = 14
    };

    DualDFF_74HC74();

    void setPin(int pin, LogicLevel level) override;
    LogicLevel getPin(int pin) const override;
    void setPower(bool on) override;
    bool isPowerOn() const override;
    double getPropagationDelay() const override;

private:
    std::map<int, LogicLevel> pinStates;
    bool powerOn;
    static constexpr double PROPAGATION_DELAY_NS = 15.0;

    // Internal stored state for Q outputs
    LogicLevel q1_state;
    LogicLevel q2_state;
    // Track last clock to detect rising edge if inputs change during propagation
    LogicLevel last_clk1;
    LogicLevel last_clk2;

    bool isInputPin(int pin) const;
    bool isAsyncPinActive(LogicLevel pre_n, LogicLevel clr_n) const;
    void updateOutputs();
};

#endif // DUAL_DFF_74HC74_H


