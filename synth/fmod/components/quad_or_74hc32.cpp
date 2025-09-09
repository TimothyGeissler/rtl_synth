/**
 * @file quad_or_74hc32.cpp
 * @brief Functional simulation of 74HC32 Quad OR Logic Gate IC
 * 
 * The 74HC32 is a quad 2-input OR gate IC in a 14-pin DIP package.
 * This class provides a functional simulation of the device including:
 * - Four independent OR gates
 * - Proper pin mapping according to datasheet
 * - Power supply and ground connections
 * - Propagation delay simulation
 * 
 * Pin Configuration (DIP-14):
 * Pin 1  - Gate 1 Input A
 * Pin 2  - Gate 1 Input B  
 * Pin 3  - Gate 1 Output Y
 * Pin 4  - Gate 2 Input A
 * Pin 5  - Gate 2 Input B
 * Pin 6  - Gate 2 Output Y
 * Pin 7  - Ground (GND)
 * Pin 8  - Gate 3 Output Y
 * Pin 9  - Gate 3 Input A
 * Pin 10 - Gate 3 Input B
 * Pin 11 - Gate 4 Output Y
 * Pin 12 - Gate 4 Input A
 * Pin 13 - Gate 4 Input B
 * Pin 14 - Power Supply (VCC)
 */

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cassert>

class QuadOR_74HC32 {
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

private:
    // Pin states (0 = LOW, 1 = HIGH, -1 = FLOATING)
    std::map<Pin, LogicLevel> pinStates;
    
    // Power supply state
    bool powerOn;
    
    // Propagation delay in nanoseconds (typical for 74HC32)
    static constexpr double PROPAGATION_DELAY_NS = 8.0;
    
    // Gate structure for internal organization
    struct Gate {
        Pin inputA;
        Pin inputB;
        Pin output;
        std::string name;
    };
    
    // Array of the four OR gates
    std::vector<Gate> gates;

public:
    /**
     * @brief Constructor - Initialize the 74HC32 IC
     */
    QuadOR_74HC32() : powerOn(false) {
        // Initialize all pins to floating state
        for (int pin = 1; pin <= 14; pin++) {
            pinStates[static_cast<Pin>(pin)] = FLOATING;
        }
        
        // Set up the four OR gates according to pinout
        gates = {
            {GATE1_A, GATE1_B, GATE1_Y, "Gate 1"},
            {GATE2_A, GATE2_B, GATE2_Y, "Gate 2"},
            {GATE3_A, GATE3_B, GATE3_Y, "Gate 3"},
            {GATE4_A, GATE4_B, GATE4_Y, "Gate 4"}
        };
        
        // Initialize power pins
        setPin(VCC, HIGH);  // Power supply
        setPin(GND, LOW);   // Ground
        powerOn = true;
    }
    
    /**
     * @brief Set the logic level of a specific pin
     * @param pin The pin number (1-14)
     * @param level The logic level (LOW, HIGH, or FLOATING)
     */
    void setPin(Pin pin, LogicLevel level) {
        assert(pin >= 1 && pin <= 14);
        pinStates[pin] = level;
        
        // If this is an input pin change, update outputs
        if (isInputPin(pin)) {
            updateOutputs();
        }
    }
    
    /**
     * @brief Get the current logic level of a specific pin
     * @param pin The pin number (1-14)
     * @return The current logic level
     */
    LogicLevel getPin(Pin pin) const {
        assert(pin >= 1 && pin <= 14);
        return pinStates.at(pin);
    }
    
    /**
     * @brief Set input pins for a specific gate
     * @param gateNumber Gate number (1-4)
     * @param inputA Logic level for input A
     * @param inputB Logic level for input B
     */
    void setGateInputs(int gateNumber, LogicLevel inputA, LogicLevel inputB) {
        assert(gateNumber >= 1 && gateNumber <= 4);
        
        const Gate& gate = gates[gateNumber - 1];
        setPin(gate.inputA, inputA);
        setPin(gate.inputB, inputB);
    }
    
    /**
     * @brief Get the output of a specific gate
     * @param gateNumber Gate number (1-4)
     * @return The output logic level
     */
    LogicLevel getGateOutput(int gateNumber) const {
        assert(gateNumber >= 1 && gateNumber <= 4);
        
        const Gate& gate = gates[gateNumber - 1];
        return getPin(gate.output);
    }
    
    /**
     * @brief Turn power on/off
     * @param on True to turn power on, false to turn off
     */
    void setPower(bool on) {
        powerOn = on;
        if (powerOn) {
            setPin(VCC, HIGH);
            setPin(GND, LOW);
        } else {
            // When power is off, all outputs go to floating state
            for (const auto& gate : gates) {
                pinStates[gate.output] = FLOATING;
            }
        }
    }
    
    /**
     * @brief Check if power is on
     * @return True if power is on
     */
    bool isPowerOn() const {
        return powerOn;
    }
    
    /**
     * @brief Get propagation delay
     * @return Propagation delay in nanoseconds
     */
    double getPropagationDelay() const {
        return PROPAGATION_DELAY_NS;
    }
    
    /**
     * @brief Print current pin states
     */
    void printPinStates() const {
        std::cout << "74HC32 Pin States:\n";
        std::cout << "==================\n";
        
        for (int pin = 1; pin <= 14; pin++) {
            Pin p = static_cast<Pin>(pin);
            std::string levelStr = logicLevelToString(getPin(p));
            std::string pinName = getPinName(p);
            
            std::cout << "Pin " << pin << " (" << pinName << "): " << levelStr << "\n";
        }
        std::cout << "\n";
    }
    
    /**
     * @brief Print gate truth table for current inputs
     */
    void printGateStates() const {
        std::cout << "74HC32 Gate States:\n";
        std::cout << "===================\n";
        
        for (size_t i = 0; i < gates.size(); i++) {
            const Gate& gate = gates[i];
            LogicLevel inputA = getPin(gate.inputA);
            LogicLevel inputB = getPin(gate.inputB);
            LogicLevel output = getPin(gate.output);
            
            std::cout << gate.name << ": ";
            std::cout << logicLevelToString(inputA) << " OR " << logicLevelToString(inputB);
            std::cout << " = " << logicLevelToString(output) << "\n";
        }
        std::cout << "\n";
    }
    
    /**
     * @brief Run a comprehensive test of all gates
     */
    void runSelfTest() {
        std::cout << "74HC32 Self Test\n";
        std::cout << "================\n";
        
        // Test all possible input combinations for each gate
        LogicLevel testInputs[] = {LOW, HIGH};
        
        for (int gateNum = 1; gateNum <= 4; gateNum++) {
            std::cout << "\nTesting " << gates[gateNum-1].name << ":\n";
            std::cout << "A\tB\tY (Expected)\tY (Actual)\tStatus\n";
            std::cout << "----------------------------------------\n";
            
            for (LogicLevel a : testInputs) {
                for (LogicLevel b : testInputs) {
                    setGateInputs(gateNum, a, b);
                    LogicLevel actual = getGateOutput(gateNum);
                    LogicLevel expected = orLogic(a, b);
                    
                    std::string status = (actual == expected) ? "PASS" : "FAIL";
                    
                    std::cout << logicLevelToString(a) << "\t" 
                              << logicLevelToString(b) << "\t"
                              << logicLevelToString(expected) << "\t\t"
                              << logicLevelToString(actual) << "\t\t"
                              << status << "\n";
                }
            }
        }
    }

private:
    /**
     * @brief Check if a pin is an input pin
     * @param pin The pin to check
     * @return True if the pin is an input
     */
    bool isInputPin(Pin pin) const {
        return (pin == GATE1_A || pin == GATE1_B ||
                pin == GATE2_A || pin == GATE2_B ||
                pin == GATE3_A || pin == GATE3_B ||
                pin == GATE4_A || pin == GATE4_B);
    }
    
    /**
     * @brief Update all output pins based on current inputs
     */
    void updateOutputs() {
        if (!powerOn) {
            return; // No outputs when power is off
        }
        
        for (const auto& gate : gates) {
            LogicLevel inputA = getPin(gate.inputA);
            LogicLevel inputB = getPin(gate.inputB);
            
            // OR logic: output is HIGH when at least one input is HIGH
            LogicLevel output = orLogic(inputA, inputB);
            pinStates[gate.output] = output;
        }
    }
    
    /**
     * @brief OR logic function
     * @param a First input
     * @param b Second input
     * @return OR result
     */
    LogicLevel orLogic(LogicLevel a, LogicLevel b) const {
        // If either input is floating, output is floating
        if (a == FLOATING || b == FLOATING) {
            return FLOATING;
        }
        
        // OR: HIGH when at least one input is HIGH
        return (a == HIGH || b == HIGH) ? HIGH : LOW;
    }
    
    /**
     * @brief Convert logic level to string
     * @param level The logic level
     * @return String representation
     */
    std::string logicLevelToString(LogicLevel level) const {
        switch (level) {
            case LOW: return "LOW (0)";
            case HIGH: return "HIGH (1)";
            case FLOATING: return "FLOATING (Z)";
            default: return "UNKNOWN";
        }
    }
    
    /**
     * @brief Get pin name for display purposes
     * @param pin The pin number
     * @return Pin name string
     */
    std::string getPinName(Pin pin) const {
        switch (pin) {
            case GATE1_A: return "Gate1_A";
            case GATE1_B: return "Gate1_B";
            case GATE1_Y: return "Gate1_Y";
            case GATE2_A: return "Gate2_A";
            case GATE2_B: return "Gate2_B";
            case GATE2_Y: return "Gate2_Y";
            case GND: return "GND";
            case GATE3_Y: return "Gate3_Y";
            case GATE3_A: return "Gate3_A";
            case GATE3_B: return "Gate3_B";
            case GATE4_Y: return "Gate4_Y";
            case GATE4_A: return "Gate4_A";
            case GATE4_B: return "Gate4_B";
            case VCC: return "VCC";
            default: return "UNKNOWN";
        }
    }
};

// Example usage and test program
int main() {
    std::cout << "74HC32 Quad OR Gate Functional Simulation\n";
    std::cout << "=========================================\n\n";
    
    // Create an instance of the 74HC32
    QuadOR_74HC32 ic;
    
    // Print initial state
    ic.printPinStates();
    
    // Test individual gates
    std::cout << "Testing individual gates:\n";
    std::cout << "========================\n";
    
    // Test Gate 1: 0 OR 0 = 0
    ic.setGateInputs(1, QuadOR_74HC32::LOW, QuadOR_74HC32::LOW);
    std::cout << "Gate 1: 0 OR 0 = " << ic.getGateOutput(1) << "\n";
    
    // Test Gate 1: 0 OR 1 = 1
    ic.setGateInputs(1, QuadOR_74HC32::LOW, QuadOR_74HC32::HIGH);
    std::cout << "Gate 1: 0 OR 1 = " << ic.getGateOutput(1) << "\n";
    
    // Test Gate 1: 1 OR 0 = 1
    ic.setGateInputs(1, QuadOR_74HC32::HIGH, QuadOR_74HC32::LOW);
    std::cout << "Gate 1: 1 OR 0 = " << ic.getGateOutput(1) << "\n";
    
    // Test Gate 1: 1 OR 1 = 1
    ic.setGateInputs(1, QuadOR_74HC32::HIGH, QuadOR_74HC32::HIGH);
    std::cout << "Gate 1: 1 OR 1 = " << ic.getGateOutput(1) << "\n\n";
    
    // Test all gates simultaneously
    std::cout << "Testing all gates simultaneously:\n";
    std::cout << "=================================\n";
    
    ic.setGateInputs(1, QuadOR_74HC32::LOW, QuadOR_74HC32::HIGH);   // Gate 1: 0 OR 1 = 1
    ic.setGateInputs(2, QuadOR_74HC32::HIGH, QuadOR_74HC32::LOW);   // Gate 2: 1 OR 0 = 1
    ic.setGateInputs(3, QuadOR_74HC32::HIGH, QuadOR_74HC32::HIGH);  // Gate 3: 1 OR 1 = 1
    ic.setGateInputs(4, QuadOR_74HC32::LOW, QuadOR_74HC32::LOW);    // Gate 4: 0 OR 0 = 0
    
    ic.printGateStates();
    
    // Test power off behavior
    std::cout << "Testing power off behavior:\n";
    std::cout << "===========================\n";
    ic.setPower(false);
    ic.printGateStates();
    
    // Turn power back on
    ic.setPower(true);
    ic.printGateStates();
    
    // Run comprehensive self test
    ic.runSelfTest();
    
    std::cout << "\nSimulation completed successfully!\n";
    std::cout << "Propagation delay: " << ic.getPropagationDelay() << " ns\n";
    
    return 0;
}
