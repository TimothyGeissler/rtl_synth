#include "dual_dff_74hc74.h"

DualDFF_74HC74::DualDFF_74HC74()
    : powerOn(false), q1_state(Component::LOW), q2_state(Component::LOW),
      last_clk1(Component::LOW), last_clk2(Component::LOW) {
    for (int pin = 1; pin <= 14; ++pin) pinStates[pin] = Component::FLOATING;
    setPin(VCC, Component::HIGH);
    setPin(GND, Component::LOW);
    // Default inactive async controls (active-low)
    pinStates[CLR1_N] = Component::HIGH;
    pinStates[PRE1_N] = Component::HIGH;
    pinStates[CLR2_N] = Component::HIGH;
    pinStates[PRE2_N] = Component::HIGH;
    powerOn = true;
}

void DualDFF_74HC74::setPin(int pin, Component::LogicLevel level) {
    pinStates[pin] = level;
    updateOutputs();
}

Component::LogicLevel DualDFF_74HC74::getPin(int pin) const {
    auto it = pinStates.find(pin);
    if (it != pinStates.end()) return it->second;
    return Component::FLOATING;
}

void DualDFF_74HC74::setPower(bool on) {
    powerOn = on;
    if (powerOn) {
        setPin(VCC, Component::HIGH);
        setPin(GND, Component::LOW);
    } else {
        pinStates[Q1] = Component::FLOATING;
        pinStates[Q1_N] = Component::FLOATING;
        pinStates[Q2] = Component::FLOATING;
        pinStates[Q2_N] = Component::FLOATING;
    }
}

bool DualDFF_74HC74::isPowerOn() const { return powerOn; }

double DualDFF_74HC74::getPropagationDelay() const { return PROPAGATION_DELAY_NS; }

bool DualDFF_74HC74::isInputPin(int pin) const {
    return pin == D1 || pin == CLK1 || pin == PRE1_N || pin == CLR1_N ||
           pin == D2 || pin == CLK2 || pin == PRE2_N || pin == CLR2_N;
}

bool DualDFF_74HC74::isAsyncPinActive(Component::LogicLevel pre_n, Component::LogicLevel clr_n) const {
    return (pre_n == Component::LOW) || (clr_n == Component::LOW);
}

void DualDFF_74HC74::updateOutputs() {
    if (!powerOn) return;

    // Read inputs
    auto pre1 = getPin(PRE1_N);
    auto clr1 = getPin(CLR1_N);
    auto d1 = getPin(D1);
    auto clk1 = getPin(CLK1);

    auto pre2 = getPin(PRE2_N);
    auto clr2 = getPin(CLR2_N);
    auto d2 = getPin(D2);
    auto clk2 = getPin(CLK2);

    // Async behavior dominates
    if (pre1 == Component::LOW && clr1 == Component::HIGH) q1_state = Component::HIGH;
    else if (clr1 == Component::LOW && pre1 == Component::HIGH) q1_state = Component::LOW;
    else {
        // Rising edge detect
        if (last_clk1 == Component::LOW && clk1 == Component::HIGH) {
            if (d1 != Component::FLOATING) q1_state = d1;
        }
    }

    if (pre2 == Component::LOW && clr2 == Component::HIGH) q2_state = Component::HIGH;
    else if (clr2 == Component::LOW && pre2 == Component::HIGH) q2_state = Component::LOW;
    else {
        if (last_clk2 == Component::LOW && clk2 == Component::HIGH) {
            if (d2 != Component::FLOATING) q2_state = d2;
        }
    }

    last_clk1 = clk1;
    last_clk2 = clk2;

    // Drive outputs
    pinStates[Q1] = q1_state;
    pinStates[Q1_N] = (q1_state == Component::FLOATING) ? Component::FLOATING : (q1_state == Component::HIGH ? Component::LOW : Component::HIGH);
    pinStates[Q2] = q2_state;
    pinStates[Q2_N] = (q2_state == Component::FLOATING) ? Component::FLOATING : (q2_state == Component::HIGH ? Component::LOW : Component::HIGH);
}


