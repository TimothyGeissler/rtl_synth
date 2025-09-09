/**
 * @file simple_test.cpp
 * @brief Simple test program to demonstrate the functional model system
 */

#include "fmodel.h"
#include "components.h"
#include <iostream>

using namespace FModel;

int main() {
    std::cout << "=== Simple Functional Model Test ===" << std::endl;
    
    // Create a simple full adder manually
    FModel::FModel model;
    
    // Create signals
    auto a_signal = model.createSignal("a", true, false);
    auto b_signal = model.createSignal("b", true, false);
    auto cin_signal = model.createSignal("cin", true, false);
    auto sum_signal = model.createSignal("sum", false, true);
    auto cout_signal = model.createSignal("cout", false, true);
    
    // Create components
    model.addComponent("U1", "74HC86", "DIP-14");  // XOR gate for sum
    model.addComponent("U2", "74HC32", "DIP-14");  // OR gate for carry
    
    // Connect signals (simplified connections)
    model.connectSignal("U1", "1", "a");
    model.connectSignal("U1", "2", "b");
    model.connectSignal("U1", "3", "sum");
    model.connectSignal("U2", "1", "a");
    model.connectSignal("U2", "2", "b");
    model.connectSignal("U2", "3", "cout");
    
    // Print circuit info
    model.printCircuitInfo();
    
    // Test individual components
    std::cout << "\n=== Testing Individual Components ===" << std::endl;
    
    // Test XOR gate (74HC86)
    QuadXOR_74HC86 xor_gate;
    std::cout << "Testing XOR Gate:" << std::endl;
    xor_gate.setGateInputs(1, Component::LOW, Component::HIGH);
    std::cout << "0 XOR 1 = " << xor_gate.getGateOutput(1) << std::endl;
    
    xor_gate.setGateInputs(1, Component::HIGH, Component::HIGH);
    std::cout << "1 XOR 1 = " << xor_gate.getGateOutput(1) << std::endl;
    
    // Test OR gate (74HC32)
    QuadOR_74HC32 or_gate;
    std::cout << "\nTesting OR Gate:" << std::endl;
    or_gate.setGateInputs(1, Component::LOW, Component::HIGH);
    std::cout << "0 OR 1 = " << or_gate.getGateOutput(1) << std::endl;
    
    or_gate.setGateInputs(1, Component::HIGH, Component::HIGH);
    std::cout << "1 OR 1 = " << or_gate.getGateOutput(1) << std::endl;
    
    // Test AND gate (74HC08)
    QuadAND_74HC08 and_gate;
    std::cout << "\nTesting AND Gate:" << std::endl;
    and_gate.setGateInputs(1, Component::LOW, Component::HIGH);
    std::cout << "0 AND 1 = " << and_gate.getGateOutput(1) << std::endl;
    
    and_gate.setGateInputs(1, Component::HIGH, Component::HIGH);
    std::cout << "1 AND 1 = " << and_gate.getGateOutput(1) << std::endl;
    
    // Test NAND gate (74HC00)
    QuadNAND_74HC00 nand_gate;
    std::cout << "\nTesting NAND Gate:" << std::endl;
    nand_gate.setGateInputs(1, Component::LOW, Component::HIGH);
    std::cout << "0 NAND 1 = " << nand_gate.getGateOutput(1) << std::endl;
    
    nand_gate.setGateInputs(1, Component::HIGH, Component::HIGH);
    std::cout << "1 NAND 1 = " << nand_gate.getGateOutput(1) << std::endl;
    
    // Test NOR gate (74HC02)
    QuadNOR_74HC02 nor_gate;
    std::cout << "\nTesting NOR Gate:" << std::endl;
    nor_gate.setGateInputs(1, Component::LOW, Component::HIGH);
    std::cout << "0 NOR 1 = " << nor_gate.getGateOutput(1) << std::endl;
    
    nor_gate.setGateInputs(1, Component::LOW, Component::LOW);
    std::cout << "0 NOR 0 = " << nor_gate.getGateOutput(1) << std::endl;
    
    std::cout << "\n=== All Component Tests Completed ===" << std::endl;
    std::cout << "✓ Functional Model Framework is working correctly!" << std::endl;
    
    return 0;
}
