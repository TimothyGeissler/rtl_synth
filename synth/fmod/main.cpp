/**
 * @file main.cpp
 * @brief Main program demonstrating the Functional Model Framework
 */

#include "fmodel.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::cout << "=== Functional Model Framework Demo ===" << std::endl;
    std::cout << "Digital Circuit Simulation using 74xx Series Components" << std::endl;
    std::cout << "========================================================" << std::endl;
    
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <netlist_file(.net)> <test_vectors_file>" << std::endl;
        std::cout << "Example: " << argv[0] << " ../netlist/full_adder.net test_vectors/full_adder_tests.txt" << std::endl;
        return 1;
    }
    
    std::string netlist_file = argv[1];
    std::string test_vectors_file = argv[2];
    
    // Create functional model
    ::FModel::FModel model;
    
    // Load netlist
    std::cout << "\n1. Loading Circuit Netlist..." << std::endl;
    if (!model.loadFromNetlist(netlist_file)) {
        std::cerr << "Failed to load netlist: " << netlist_file << std::endl;
        return 1;
    }
    
    // Load test vectors
    std::cout << "\n2. Loading Test Vectors..." << std::endl;
    if (!model.loadTestVectors(test_vectors_file)) {
        std::cerr << "Failed to load test vectors: " << test_vectors_file << std::endl;
        return 1;
    }
    
    // Print initial circuit state
    std::cout << "\n3. Initial Circuit State..." << std::endl;
    model.printCircuitState();
    
    // Run simulation
    std::cout << "\n4. Running Simulation..." << std::endl;
    bool simulation_success = model.simulate();
    
    // Print final results
    std::cout << "\n5. Simulation Results..." << std::endl;
    if (simulation_success) {
        std::cout << "✓ All tests PASSED!" << std::endl;
    } else {
        std::cout << "✗ Some tests FAILED!" << std::endl;
    }
    
    std::cout << "\n=== Demo Complete ===" << std::endl;
    
    return simulation_success ? 0 : 1;
}
