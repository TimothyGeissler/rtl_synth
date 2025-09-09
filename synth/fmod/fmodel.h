/**
 * @file fmodel.h
 * @brief Functional Model Framework for Digital Circuit Simulation
 * 
 * This header defines the main FModel class that can construct circuit models
 * from netlists and run stimuli through them to test functionality.
 */

#ifndef FMODEL_H
#define FMODEL_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <iostream>
#include <fstream>

// Forward declarations for component classes
class Component;
class QuadAND_74HC08;
class QuadOR_74HC32;
class QuadNAND_74HC00;
class QuadNOR_74HC02;
class QuadXOR_74HC86;

namespace FModel {

/**
 * @brief Logic level enumeration
 */
enum class LogicLevel {
    LOW = 0,
    HIGH = 1,
    FLOATING = -1
};

/**
 * @brief Signal class representing a wire in the circuit
 */
class Signal {
public:
    std::string name;
    LogicLevel level;
    bool is_input;
    bool is_output;
    bool is_internal;
    
    Signal(const std::string& name, bool is_input = false, bool is_output = false) 
        : name(name), level(LogicLevel::FLOATING), is_input(is_input), 
          is_output(is_output), is_internal(!is_input && !is_output) {}
    
    void setLevel(LogicLevel new_level) { level = new_level; }
    LogicLevel getLevel() const { return level; }
    std::string getName() const { return name; }
};

/**
 * @brief Component instance in the circuit
 */
class ComponentInstance {
public:
    std::string instance_id;
    std::string part_number;
    std::string package;
    std::map<std::string, std::string> pin_assignments;
    std::vector<std::map<std::string, std::string>> gates;
    
    // Component object (polymorphic)
    std::shared_ptr<Component> component;
    
    ComponentInstance(const std::string& id, const std::string& part, const std::string& pkg)
        : instance_id(id), part_number(part), package(pkg) {}
    
    void addPinAssignment(const std::string& pin, const std::string& signal) {
        pin_assignments[pin] = signal;
    }
    
    void addGate(const std::map<std::string, std::string>& gate) {
        gates.push_back(gate);
    }
};

/**
 * @brief Test vector for stimuli
 */
class TestVector {
public:
    std::map<std::string, LogicLevel> inputs;
    std::map<std::string, LogicLevel> expected_outputs;
    std::string description;
    
    TestVector(const std::string& desc = "") : description(desc) {}
    
    void addInput(const std::string& signal, LogicLevel level) {
        inputs[signal] = level;
    }
    
    void addExpectedOutput(const std::string& signal, LogicLevel level) {
        expected_outputs[signal] = level;
    }
};

/**
 * @brief Main Functional Model class
 */
class FModel {
private:
    std::string module_name;
    std::vector<std::shared_ptr<Signal>> signals;
    std::vector<std::shared_ptr<ComponentInstance>> components;
    std::map<std::string, std::shared_ptr<Signal>> signal_map;
    std::map<std::string, std::shared_ptr<ComponentInstance>> component_map;
    
    // Component factory functions
    std::map<std::string, std::function<std::shared_ptr<Component>()>> component_factories;
    
    // Simulation state
    bool simulation_ready;
    std::vector<TestVector> test_vectors;
    
public:
    FModel();
    ~FModel();
    
    // Circuit construction
    bool loadFromNetlist(const std::string& netlist_file);
    bool addComponent(const std::string& instance_id, const std::string& part_number, 
                     const std::string& package = "DIP-14");
    bool connectSignal(const std::string& instance_id, const std::string& pin, 
                      const std::string& signal_name);
    
    // Signal management
    std::shared_ptr<Signal> getSignal(const std::string& name);
    std::shared_ptr<Signal> createSignal(const std::string& name, bool is_input = false, 
                                        bool is_output = false);
    void setSignalLevel(const std::string& signal_name, LogicLevel level);
    LogicLevel getSignalLevel(const std::string& signal_name) const;
    
    // Stimuli management
    bool loadTestVectors(const std::string& test_file);
    void addTestVector(const TestVector& test_vector);
    void clearTestVectors();
    
    // Simulation
    bool simulate();
    bool simulateTestVector(const TestVector& test_vector);
    void printCircuitState() const;
    void printTestResults() const;
    
    // Utility functions
    std::string logicLevelToString(LogicLevel level) const;
    LogicLevel stringToLogicLevel(const std::string& str) const;
    void printCircuitInfo() const;
    
private:
    // Internal helper functions
    void initializeComponentFactories();
    std::shared_ptr<Component> createComponent(const std::string& part_number);
    bool parseNetlistFile(const std::string& filename);
    bool parseTestVectorFile(const std::string& filename);
    void updateComponentOutputs();
    void propagateSignals();
    bool validateCircuit() const;
    void resetCircuit();
};

} // namespace FModel

#endif // FMODEL_H
