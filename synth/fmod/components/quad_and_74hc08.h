/**
 * @file quad_and_74hc08.h
 * @brief Header for 74HC08 Quad AND Logic Gate IC simulation
 */

#ifndef QUAD_AND_74HC08_H
#define QUAD_AND_74HC08_H

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cassert>

class QuadAND_74HC08 {
public:
    // Pin definitions
    enum Pin {
        GATE1_A = 1,    // Gate 1 Input A
        GATE1_B = 2,    // Gate 1 Input B
        GATE1_Y = 3,    // Gate 1 Output Y
        GATE2_A = 4,    // Gate 2 Input A
        GATE2_B = 5,    // Gate 2 Input B
        GATE2_Y = 6,    // Gate 2 Output Y
        GND = 7,        // Ground
        GATE3_Y = 8,    // Gate 3 Output Y
        GATE3_A = 9,    // Gate 3 Input A
        GATE3_B = 10,   // Gate 3 Input B
        GATE4_Y = 11,   // Gate 4 Output Y
        GATE4_A = 12,   // Gate 4 Input A
        GATE4_B = 13,   // Gate 4 Input B
        VCC = 14        // Power Supply
    };

    // Logic levels
    enum LogicLevel {
        LOW = 0,
        HIGH = 1,
        FLOATING = -1   // High impedance/undefined
    };

    // Constructor
    QuadAND_74HC08();
    
    // Pin operations
    void setPin(Pin pin, LogicLevel level);
    LogicLevel getPin(Pin pin) const;
    
    // Gate operations
    void setGateInputs(int gateNumber, LogicLevel inputA, LogicLevel inputB);
    LogicLevel getGateOutput(int gateNumber) const;
    
    // Power management
    void setPower(bool on);
    bool isPowerOn() const;
    
    // Utility functions
    double getPropagationDelay() const;
    void printPinStates() const;
    void printGateStates() const;
    void runSelfTest();

private:
    // Pin states
    std::map<Pin, LogicLevel> pinStates;
    bool powerOn;
    static constexpr double PROPAGATION_DELAY_NS = 8.0;
    
    // Gate structure
    struct Gate {
        Pin inputA;
        Pin inputB;
        Pin output;
        std::string name;
    };
    std::vector<Gate> gates;
    
    // Helper functions
    bool isInputPin(Pin pin) const;
    void updateOutputs();
    LogicLevel andLogic(LogicLevel a, LogicLevel b) const;
    std::string logicLevelToString(LogicLevel level) const;
    std::string getPinName(Pin pin) const;
};

#endif // QUAD_AND_74HC08_H
