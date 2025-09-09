#include "quad_not_74hc04.h"

HexInverter_74HC04::HexInverter_74HC04() : powerOn(false) {
    for (int pin = 1; pin <= 14; pin++) {
        pinStates[pin] = Component::FLOATING;
    }
    gates = {
        {GATE1_A, GATE1_Y},
        {GATE2_A, GATE2_Y},
        {GATE3_A, GATE3_Y},
        {GATE4_A, GATE4_Y},
        {GATE5_A, GATE5_Y},
        {GATE6_A, GATE6_Y}
    };
    setPin(VCC, Component::HIGH);
    setPin(GND, Component::LOW);
    powerOn = true;
}

void HexInverter_74HC04::setPin(int pin, Component::LogicLevel level) {
    pinStates[pin] = level;
    if (isInputPin(pin)) {
        updateOutputs();
    }
}

Component::LogicLevel HexInverter_74HC04::getPin(int pin) const {
    auto it = pinStates.find(pin);
    if (it != pinStates.end()) return it->second;
    return Component::FLOATING;
}

void HexInverter_74HC04::setPower(bool on) {
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

bool HexInverter_74HC04::isPowerOn() const { return powerOn; }

double HexInverter_74HC04::getPropagationDelay() const { return PROPAGATION_DELAY_NS; }

bool HexInverter_74HC04::isInputPin(int pin) const {
    return (pin == GATE1_A || pin == GATE2_A || pin == GATE3_A || pin == GATE4_A || pin == GATE5_A || pin == GATE6_A);
}

void HexInverter_74HC04::setGateInput(int gateNumber, Component::LogicLevel inputA) {
    const Gate& gate = gates[gateNumber - 1];
    setPin(gate.inputA, inputA);
}

Component::LogicLevel HexInverter_74HC04::getGateOutput(int gateNumber) const {
    const Gate& gate = gates[gateNumber - 1];
    return getPin(gate.output);
}

void HexInverter_74HC04::updateOutputs() {
    if (!powerOn) return;
    for (const auto& gate : gates) {
        Component::LogicLevel inputA = getPin(gate.inputA);
        Component::LogicLevel output = notLogic(inputA);
        pinStates[gate.output] = output;
    }
}

Component::LogicLevel HexInverter_74HC04::notLogic(Component::LogicLevel a) const {
    if (a == Component::FLOATING) return Component::FLOATING;
    return (a == Component::HIGH) ? Component::LOW : Component::HIGH;
}
