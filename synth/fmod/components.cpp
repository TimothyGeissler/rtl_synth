/**
 * @file components.cpp
 * @brief Implementation of all 74xx series components
 */

#include "components.h"

// 74HC08 Quad AND Gate Implementation
QuadAND_74HC08::QuadAND_74HC08() : powerOn(false) {
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

void QuadAND_74HC08::setPin(int pin, Component::LogicLevel level) {
    assert(pin >= 1 && pin <= 14);
    pinStates[pin] = level;
    if (isInputPin(pin)) {
        updateOutputs();
    }
}

Component::LogicLevel QuadAND_74HC08::getPin(int pin) const {
    assert(pin >= 1 && pin <= 14);
    return pinStates.at(pin);
}

void QuadAND_74HC08::setGateInputs(int gateNumber, Component::LogicLevel inputA, Component::LogicLevel inputB) {
    assert(gateNumber >= 1 && gateNumber <= 4);
    const Gate& gate = gates[gateNumber - 1];
    setPin(gate.inputA, inputA);
    setPin(gate.inputB, inputB);
}

Component::LogicLevel QuadAND_74HC08::getGateOutput(int gateNumber) const {
    assert(gateNumber >= 1 && gateNumber <= 4);
    const Gate& gate = gates[gateNumber - 1];
    return getPin(gate.output);
}

void QuadAND_74HC08::setPower(bool on) {
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

bool QuadAND_74HC08::isPowerOn() const {
    return powerOn;
}

double QuadAND_74HC08::getPropagationDelay() const {
    return PROPAGATION_DELAY_NS;
}

bool QuadAND_74HC08::isInputPin(int pin) const {
    return (pin == GATE1_A || pin == GATE1_B ||
            pin == GATE2_A || pin == GATE2_B ||
            pin == GATE3_A || pin == GATE3_B ||
            pin == GATE4_A || pin == GATE4_B);
}

void QuadAND_74HC08::updateOutputs() {
    if (!powerOn) return;
    
    for (const auto& gate : gates) {
        Component::LogicLevel inputA = getPin(gate.inputA);
        Component::LogicLevel inputB = getPin(gate.inputB);
        Component::LogicLevel output = andLogic(inputA, inputB);
        pinStates[gate.output] = output;
    }
}

Component::LogicLevel QuadAND_74HC08::andLogic(Component::LogicLevel a, Component::LogicLevel b) const {
    if (a == Component::FLOATING || b == Component::FLOATING) {
        return Component::FLOATING;
    }
    return (a == Component::HIGH && b == Component::HIGH) ? Component::HIGH : Component::LOW;
}

// 74HC32 Quad OR Gate Implementation
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
    assert(pin >= 1 && pin <= 14);
    pinStates[pin] = level;
    if (isInputPin(pin)) {
        updateOutputs();
    }
}

Component::LogicLevel QuadOR_74HC32::getPin(int pin) const {
    assert(pin >= 1 && pin <= 14);
    return pinStates.at(pin);
}

void QuadOR_74HC32::setGateInputs(int gateNumber, Component::LogicLevel inputA, Component::LogicLevel inputB) {
    assert(gateNumber >= 1 && gateNumber <= 4);
    const Gate& gate = gates[gateNumber - 1];
    setPin(gate.inputA, inputA);
    setPin(gate.inputB, inputB);
}

Component::LogicLevel QuadOR_74HC32::getGateOutput(int gateNumber) const {
    assert(gateNumber >= 1 && gateNumber <= 4);
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

// 74HC00 Quad NAND Gate Implementation
QuadNAND_74HC00::QuadNAND_74HC00() : powerOn(false) {
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

void QuadNAND_74HC00::setPin(int pin, Component::LogicLevel level) {
    assert(pin >= 1 && pin <= 14);
    pinStates[pin] = level;
    if (isInputPin(pin)) {
        updateOutputs();
    }
}

Component::LogicLevel QuadNAND_74HC00::getPin(int pin) const {
    assert(pin >= 1 && pin <= 14);
    return pinStates.at(pin);
}

void QuadNAND_74HC00::setGateInputs(int gateNumber, Component::LogicLevel inputA, Component::LogicLevel inputB) {
    assert(gateNumber >= 1 && gateNumber <= 4);
    const Gate& gate = gates[gateNumber - 1];
    setPin(gate.inputA, inputA);
    setPin(gate.inputB, inputB);
}

Component::LogicLevel QuadNAND_74HC00::getGateOutput(int gateNumber) const {
    assert(gateNumber >= 1 && gateNumber <= 4);
    const Gate& gate = gates[gateNumber - 1];
    return getPin(gate.output);
}

void QuadNAND_74HC00::setPower(bool on) {
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

bool QuadNAND_74HC00::isPowerOn() const {
    return powerOn;
}

double QuadNAND_74HC00::getPropagationDelay() const {
    return PROPAGATION_DELAY_NS;
}

bool QuadNAND_74HC00::isInputPin(int pin) const {
    return (pin == GATE1_A || pin == GATE1_B ||
            pin == GATE2_A || pin == GATE2_B ||
            pin == GATE3_A || pin == GATE3_B ||
            pin == GATE4_A || pin == GATE4_B);
}

void QuadNAND_74HC00::updateOutputs() {
    if (!powerOn) return;
    
    for (const auto& gate : gates) {
        Component::LogicLevel inputA = getPin(gate.inputA);
        Component::LogicLevel inputB = getPin(gate.inputB);
        Component::LogicLevel output = nandLogic(inputA, inputB);
        pinStates[gate.output] = output;
    }
}

Component::LogicLevel QuadNAND_74HC00::nandLogic(Component::LogicLevel a, Component::LogicLevel b) const {
    if (a == Component::FLOATING || b == Component::FLOATING) {
        return Component::FLOATING;
    }
    return (a == Component::HIGH && b == Component::HIGH) ? Component::LOW : Component::HIGH;
}

// 74HC02 Quad NOR Gate Implementation
QuadNOR_74HC02::QuadNOR_74HC02() : powerOn(false) {
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

void QuadNOR_74HC02::setPin(int pin, Component::LogicLevel level) {
    assert(pin >= 1 && pin <= 14);
    pinStates[pin] = level;
    if (isInputPin(pin)) {
        updateOutputs();
    }
}

Component::LogicLevel QuadNOR_74HC02::getPin(int pin) const {
    assert(pin >= 1 && pin <= 14);
    return pinStates.at(pin);
}

void QuadNOR_74HC02::setGateInputs(int gateNumber, Component::LogicLevel inputA, Component::LogicLevel inputB) {
    assert(gateNumber >= 1 && gateNumber <= 4);
    const Gate& gate = gates[gateNumber - 1];
    setPin(gate.inputA, inputA);
    setPin(gate.inputB, inputB);
}

Component::LogicLevel QuadNOR_74HC02::getGateOutput(int gateNumber) const {
    assert(gateNumber >= 1 && gateNumber <= 4);
    const Gate& gate = gates[gateNumber - 1];
    return getPin(gate.output);
}

void QuadNOR_74HC02::setPower(bool on) {
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

bool QuadNOR_74HC02::isPowerOn() const {
    return powerOn;
}

double QuadNOR_74HC02::getPropagationDelay() const {
    return PROPAGATION_DELAY_NS;
}

bool QuadNOR_74HC02::isInputPin(int pin) const {
    return (pin == GATE1_A || pin == GATE1_B ||
            pin == GATE2_A || pin == GATE2_B ||
            pin == GATE3_A || pin == GATE3_B ||
            pin == GATE4_A || pin == GATE4_B);
}

void QuadNOR_74HC02::updateOutputs() {
    if (!powerOn) return;
    
    for (const auto& gate : gates) {
        Component::LogicLevel inputA = getPin(gate.inputA);
        Component::LogicLevel inputB = getPin(gate.inputB);
        Component::LogicLevel output = norLogic(inputA, inputB);
        pinStates[gate.output] = output;
    }
}

Component::LogicLevel QuadNOR_74HC02::norLogic(Component::LogicLevel a, Component::LogicLevel b) const {
    if (a == Component::FLOATING || b == Component::FLOATING) {
        return Component::FLOATING;
    }
    return (a == Component::LOW && b == Component::LOW) ? Component::HIGH : Component::LOW;
}

// 74HC86 Quad XOR Gate Implementation
QuadXOR_74HC86::QuadXOR_74HC86() : powerOn(false) {
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

void QuadXOR_74HC86::setPin(int pin, Component::LogicLevel level) {
    assert(pin >= 1 && pin <= 14);
    pinStates[pin] = level;
    if (isInputPin(pin)) {
        updateOutputs();
    }
}

Component::LogicLevel QuadXOR_74HC86::getPin(int pin) const {
    assert(pin >= 1 && pin <= 14);
    return pinStates.at(pin);
}

void QuadXOR_74HC86::setGateInputs(int gateNumber, Component::LogicLevel inputA, Component::LogicLevel inputB) {
    assert(gateNumber >= 1 && gateNumber <= 4);
    const Gate& gate = gates[gateNumber - 1];
    setPin(gate.inputA, inputA);
    setPin(gate.inputB, inputB);
}

Component::LogicLevel QuadXOR_74HC86::getGateOutput(int gateNumber) const {
    assert(gateNumber >= 1 && gateNumber <= 4);
    const Gate& gate = gates[gateNumber - 1];
    return getPin(gate.output);
}

void QuadXOR_74HC86::setPower(bool on) {
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

bool QuadXOR_74HC86::isPowerOn() const {
    return powerOn;
}

double QuadXOR_74HC86::getPropagationDelay() const {
    return PROPAGATION_DELAY_NS;
}

bool QuadXOR_74HC86::isInputPin(int pin) const {
    return (pin == GATE1_A || pin == GATE1_B ||
            pin == GATE2_A || pin == GATE2_B ||
            pin == GATE3_A || pin == GATE3_B ||
            pin == GATE4_A || pin == GATE4_B);
}

void QuadXOR_74HC86::updateOutputs() {
    if (!powerOn) return;
    
    for (const auto& gate : gates) {
        Component::LogicLevel inputA = getPin(gate.inputA);
        Component::LogicLevel inputB = getPin(gate.inputB);
        Component::LogicLevel output = xorLogic(inputA, inputB);
        pinStates[gate.output] = output;
    }
}

Component::LogicLevel QuadXOR_74HC86::xorLogic(Component::LogicLevel a, Component::LogicLevel b) const {
    if (a == Component::FLOATING || b == Component::FLOATING) {
        return Component::FLOATING;
    }
    return (a != b) ? Component::HIGH : Component::LOW;
}
