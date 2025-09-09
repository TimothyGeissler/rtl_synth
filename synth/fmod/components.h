/**
 * @file components.h
 * @brief All 74xx series component declarations and implementations
 */

#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cassert>

// Base component interface
class Component {
public:
    enum LogicLevel {
        LOW = 0,
        HIGH = 1,
        FLOATING = -1
    };
    
    virtual ~Component() = default;
    virtual void setPin(int pin, LogicLevel level) = 0;
    virtual LogicLevel getPin(int pin) const = 0;
    virtual void setPower(bool on) = 0;
    virtual bool isPowerOn() const = 0;
    virtual double getPropagationDelay() const = 0;
};

// 74HC08 Quad AND Gate
class QuadAND_74HC08 : public Component {
public:
    enum Pin {
        GATE1_A = 1, GATE1_B = 2, GATE1_Y = 3,
        GATE2_A = 4, GATE2_B = 5, GATE2_Y = 6,
        GND = 7, GATE3_Y = 8, GATE3_A = 9, GATE3_B = 10,
        GATE4_Y = 11, GATE4_A = 12, GATE4_B = 13, VCC = 14
    };

    QuadAND_74HC08();
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
    LogicLevel andLogic(LogicLevel a, LogicLevel b) const;
};

// 74HC32 Quad OR Gate
class QuadOR_74HC32 : public Component {
public:
    enum Pin {
        GATE1_A = 1, GATE1_B = 2, GATE1_Y = 3,
        GATE2_A = 4, GATE2_B = 5, GATE2_Y = 6,
        GND = 7, GATE3_Y = 8, GATE3_A = 9, GATE3_B = 10,
        GATE4_Y = 11, GATE4_A = 12, GATE4_B = 13, VCC = 14
    };

    QuadOR_74HC32();
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
    LogicLevel orLogic(LogicLevel a, LogicLevel b) const;
};

// 74HC00 Quad NAND Gate
class QuadNAND_74HC00 : public Component {
public:
    enum Pin {
        GATE1_A = 1, GATE1_B = 2, GATE1_Y = 3,
        GATE2_A = 4, GATE2_B = 5, GATE2_Y = 6,
        GND = 7, GATE3_Y = 8, GATE3_A = 9, GATE3_B = 10,
        GATE4_Y = 11, GATE4_A = 12, GATE4_B = 13, VCC = 14
    };

    QuadNAND_74HC00();
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
    LogicLevel nandLogic(LogicLevel a, LogicLevel b) const;
};

// 74HC02 Quad NOR Gate
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

// 74HC86 Quad XOR Gate
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

#endif // COMPONENTS_H
