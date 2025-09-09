/**
 * @file quad_or_74hc32.cpp
 */

#include "quad_or_74hc32.h"

QuadOR_74HC32::QuadOR_74HC32() : powerOn(false) {
    for (int pin = 1; pin <= 14; pin++) {
        pinStates[pin] = Component::FLOATING;
    }
    gates = {
        {GATE1_A, GATE1_B, GATE1_Y, "Gate 1"},
        {GATE2_A, GATE2_B, GATE2_Y, "Gate 2"},
        {GATE3_A, GATE3_B, GATE3_Y, "Gate 3"},
        {GATE4_A, GATE4_B, GATE4_Y, "Gate 4"}
    };
    setPin(VCC, Component::HIGH);
    setPin(GND, Component::LOW);
    powerOn = true;
}

void QuadOR_74HC32::setPin(int pin, Component::LogicLevel level) {
    pinStates[pin] = level;
    if (isInputPin(pin)) {
        updateOutputs();
    }
}

Component::LogicLevel QuadOR_74HC32::getPin(int pin) const {
    return pinStates.at(pin);
}

void QuadOR_74HC32::setGateInputs(int gateNumber, Component::LogicLevel inputA, Component::LogicLevel inputB) {
    const Gate& gate = gates[gateNumber - 1];
    setPin(gate.inputA, inputA);
    setPin(gate.inputB, inputB);
}

Component::LogicLevel QuadOR_74HC32::getGateOutput(int gateNumber) const {
    const Gate& gate = gates[gateNumber - 1];
    return getPin(gate.output);
}

void QuadOR_74HC32::setPower(bool on) {
    powerOn = on;
    if (powerOn) {
        setPin(VCC, Component::HIGH);
        setPin(GND, Component::LOW);
    } else {
        for (const auto& gate : gates) {
            pinStates[gate.output] = Component::FLOATING;
        }
    }
}

bool QuadOR_74HC32::isPowerOn() const {
    return powerOn;
}

double QuadOR_74HC32::getPropagationDelay() const {
    return PROPAGATION_DELAY_NS;
}

bool QuadOR_74HC32::isInputPin(int pin) const {
    return (pin == GATE1_A || pin == GATE1_B ||
            pin == GATE2_A || pin == GATE2_B ||
            pin == GATE3_A || pin == GATE3_B ||
            pin == GATE4_A || pin == GATE4_B);
}

void QuadOR_74HC32::updateOutputs() {
    if (!powerOn) return;
    for (const auto& gate : gates) {
        Component::LogicLevel inputA = getPin(gate.inputA);
        Component::LogicLevel inputB = getPin(gate.inputB);
        Component::LogicLevel output = orLogic(inputA, inputB);
        pinStates[gate.output] = output;
    }
}

Component::LogicLevel QuadOR_74HC32::orLogic(Component::LogicLevel a, Component::LogicLevel b) const {
    if (a == Component::FLOATING || b == Component::FLOATING) {
        return Component::FLOATING;
    }
    return (a == Component::HIGH || b == Component::HIGH) ? Component::HIGH : Component::LOW;
}


