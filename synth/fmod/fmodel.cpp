/**
 * @file fmodel.cpp
 * @brief Implementation of the Functional Model Framework
 */

#include "fmodel.h"
#include "components.h"

namespace FModel {

FModel::FModel() : simulation_ready(false) {
    initializeComponentFactories();
}

FModel::~FModel() {
    // Cleanup handled by smart pointers
}

void FModel::initializeComponentFactories() {
    component_factories["74HC08"] = []() -> std::shared_ptr<Component> {
        return std::make_shared<QuadAND_74HC08>();
    };
    
    component_factories["74HC32"] = []() -> std::shared_ptr<Component> {
        return std::make_shared<QuadOR_74HC32>();
    };
    
    component_factories["74HC00"] = []() -> std::shared_ptr<Component> {
        return std::make_shared<QuadNAND_74HC00>();
    };
    
    component_factories["74HC02"] = []() -> std::shared_ptr<Component> {
        return std::make_shared<QuadNOR_74HC02>();
    };
    
    component_factories["74HC86"] = []() -> std::shared_ptr<Component> {
        return std::make_shared<QuadXOR_74HC86>();
    };
    component_factories["74HC04"] = []() -> std::shared_ptr<Component> {
        return std::make_shared<HexInverter_74HC04>();
    };
    component_factories["74HC74"] = []() -> std::shared_ptr<Component> {
        return std::make_shared<DualDFF_74HC74>();
    };
}

bool FModel::loadFromNetlist(const std::string& netlist_file) {
    std::cout << "Loading netlist from: " << netlist_file << std::endl;
    
    if (!parseNetlistFile(netlist_file)) {
        std::cerr << "Failed to parse netlist file: " << netlist_file << std::endl;
        return false;
    }
    
    simulation_ready = validateCircuit();
    if (simulation_ready) {
        std::cout << "Circuit loaded and validated successfully!" << std::endl;
        printCircuitInfo();
    } else {
        std::cerr << "Circuit validation failed!" << std::endl;
    }
    
    return simulation_ready;
}

bool FModel::parseNetlistFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open netlist file: " << filename << std::endl;
        return false;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    
    // Dispatch based on file extension: .net (KiCad), else assume json (legacy)
    if (filename.size() >= 4 && filename.substr(filename.size() - 4) == ".net") {
        return parseKiCadNetlist(content);
    }

    // Parse module name
    size_t module_pos = content.find("\"module_name\"");
    if (module_pos != std::string::npos) {
        size_t colon_pos = content.find(":", module_pos);
        size_t quote1 = content.find("\"", colon_pos);
        size_t quote2 = content.find("\"", quote1 + 1);
        module_name = content.substr(quote1 + 1, quote2 - quote1 - 1);
    }
    
    // Parse inputs
    size_t inputs_pos = content.find("\"inputs\"");
    if (inputs_pos != std::string::npos) {
        size_t bracket_start = content.find("[", inputs_pos);
        size_t bracket_end = content.find("]", bracket_start);
        
        if (bracket_start != std::string::npos && bracket_end != std::string::npos) {
            std::string inputs_section = content.substr(bracket_start + 1, bracket_end - bracket_start - 1);
            
            // Find all input names
            size_t pos = 0;
            while ((pos = inputs_section.find("\"name\"", pos)) != std::string::npos) {
                size_t colon_pos = inputs_section.find(":", pos);
                size_t quote1 = inputs_section.find("\"", colon_pos);
                size_t quote2 = inputs_section.find("\"", quote1 + 1);
                std::string input_name = inputs_section.substr(quote1 + 1, quote2 - quote1 - 1);
                createSignal(input_name, true, false);
                pos = quote2;
            }
        }
    }
    
    // Parse outputs
    size_t outputs_pos = content.find("\"outputs\"");
    if (outputs_pos != std::string::npos) {
        size_t bracket_start = content.find("[", outputs_pos);
        size_t bracket_end = content.find("]", bracket_start);
        
        if (bracket_start != std::string::npos && bracket_end != std::string::npos) {
            std::string outputs_section = content.substr(bracket_start + 1, bracket_end - bracket_start - 1);
            
            // Find all output names
            size_t pos = 0;
            while ((pos = outputs_section.find("\"name\"", pos)) != std::string::npos) {
                size_t colon_pos = outputs_section.find(":", pos);
                size_t quote1 = outputs_section.find("\"", colon_pos);
                size_t quote2 = outputs_section.find("\"", quote1 + 1);
                std::string output_name = outputs_section.substr(quote1 + 1, quote2 - quote1 - 1);
                createSignal(output_name, false, true);
                pos = quote2;
            }
        }
    }
    
    // Parse IC instances
    size_t instances_pos = content.find("\"ic_instances\"");
    if (instances_pos != std::string::npos) {
        size_t bracket_start = content.find("[", instances_pos);
        size_t bracket_end = content.find("]", bracket_start);
        
        if (bracket_start != std::string::npos && bracket_end != std::string::npos) {
            std::string instances_section = content.substr(bracket_start + 1, bracket_end - bracket_start - 1);
            
            // Find all instances
            size_t pos = 0;
            while ((pos = instances_section.find("\"instance_id\"", pos)) != std::string::npos) {
                // Extract instance ID
                size_t colon_pos = instances_section.find(":", pos);
                size_t quote1 = instances_section.find("\"", colon_pos);
                size_t quote2 = instances_section.find("\"", quote1 + 1);
                std::string instance_id = instances_section.substr(quote1 + 1, quote2 - quote1 - 1);
                
                // Extract part number
                size_t part_pos = instances_section.find("\"part_number\"", pos);
                size_t part_colon = instances_section.find(":", part_pos);
                size_t part_quote1 = instances_section.find("\"", part_colon);
                size_t part_quote2 = instances_section.find("\"", part_quote1 + 1);
                std::string part_number = instances_section.substr(part_quote1 + 1, part_quote2 - part_quote1 - 1);
                
                // Extract package
                size_t pkg_pos = instances_section.find("\"package\"", pos);
                size_t pkg_colon = instances_section.find(":", pkg_pos);
                size_t pkg_quote1 = instances_section.find("\"", pkg_colon);
                size_t pkg_quote2 = instances_section.find("\"", pkg_quote1 + 1);
                std::string package = instances_section.substr(pkg_quote1 + 1, pkg_quote2 - pkg_quote1 - 1);
                
                // Create component
                addComponent(instance_id, part_number, package);
                
                // Extract pin assignments
                size_t pin_pos = instances_section.find("\"pin_assignments\"", pos);
                if (pin_pos != std::string::npos) {
                    size_t pin_brace_start = instances_section.find("{", pin_pos);
                    size_t pin_brace_end = instances_section.find("}", pin_brace_start);
                    
                    if (pin_brace_start != std::string::npos && pin_brace_end != std::string::npos) {
                        std::string pin_section = instances_section.substr(pin_brace_start + 1, pin_brace_end - pin_brace_start - 1);
                        
                        // Parse pin assignments
                        size_t pin_parse_pos = 0;
                        while ((pin_parse_pos = pin_section.find("\"", pin_parse_pos)) != std::string::npos) {
                            size_t pin_quote1 = pin_parse_pos;
                            size_t pin_quote2 = pin_section.find("\"", pin_quote1 + 1);
                            std::string pin_num = pin_section.substr(pin_quote1 + 1, pin_quote2 - pin_quote1 - 1);
                            
                            size_t colon_pos = pin_section.find(":", pin_quote2);
                            size_t signal_quote1 = pin_section.find("\"", colon_pos);
                            size_t signal_quote2 = pin_section.find("\"", signal_quote1 + 1);
                            std::string signal_name = pin_section.substr(signal_quote1 + 1, signal_quote2 - signal_quote1 - 1);
                            
                            connectSignal(instance_id, pin_num, signal_name);
                            pin_parse_pos = signal_quote2;
                        }
                    }
                }
                
                pos = instances_section.find("\"instance_id\"", pos + 1);
            }
        }
    }
    
    return true;
}

bool FModel::parseKiCadNetlist(const std::string& content) {
    // Extremely lightweight S-expression-ish parser tailored to the sample .net
    // Extract components U1/U2/U3 and their part numbers (value)
    // Then extract all nets with (node (ref Ui) (pin N)) and connect pins to net name

    // Components
    size_t comps_pos = content.find("(components");
    if (comps_pos == std::string::npos) return false;
    size_t nets_pos = content.find("(nets", comps_pos);
    std::string comps_section = content.substr(comps_pos, nets_pos - comps_pos);

    size_t comp_pos = 0;
    while ((comp_pos = comps_section.find("(comp (ref ", comp_pos)) != std::string::npos) {
        size_t ref_start = comp_pos + std::string("(comp (ref ").size();
        size_t ref_end = comps_section.find(")", ref_start);
        if (ref_end == std::string::npos) break;
        std::string instance_id = comps_section.substr(ref_start, ref_end - ref_start);

        size_t next_comp = comps_section.find("(comp (ref ", ref_end);
        size_t comp_block_end = (next_comp == std::string::npos) ? comps_section.size() : next_comp;
        std::string comp_block = comps_section.substr(ref_end, comp_block_end - ref_end);

        size_t value_pos = comp_block.find("(value ");
        if (value_pos == std::string::npos) { comp_pos = comp_block_end; continue; }
        size_t value_start = value_pos + std::string("(value ").size();
        size_t value_end = comp_block.find(")", value_start);
        if (value_end == std::string::npos) { comp_pos = comp_block_end; continue; }
        std::string part_number = comp_block.substr(value_start, value_end - value_start);

        // Only add known logic ICs; skip connectors and others
        if (part_number.rfind("74", 0) == 0) {
            addComponent(instance_id, part_number, "DIP-14");
        }
        comp_pos = comp_block_end;
    }

    // Nets
    if (nets_pos == std::string::npos) return true;
    size_t nets_end = content.find("\n)\n", nets_pos);
    std::string nets_section = content.substr(nets_pos, (nets_end == std::string::npos ? content.size() : nets_end) - nets_pos);

    size_t net_pos = 0;
    while ((net_pos = nets_section.find("(net ", net_pos)) != std::string::npos) {
        // Net name
        size_t name_pos = nets_section.find("(name \"", net_pos);
        if (name_pos == std::string::npos) break;
        size_t name_start = name_pos + 7;
        size_t name_end = nets_section.find("\"", name_start);
        std::string net_name = nets_section.substr(name_start, name_end - name_start);
        if (signal_map.find(net_name) == signal_map.end()) {
            createSignal(net_name, false, false);
        }

        // Nodes
        size_t node_pos = name_end;
        while ((node_pos = nets_section.find("(node (ref ", node_pos)) != std::string::npos) {
            size_t ref_start = node_pos + 11;
            size_t ref_end = nets_section.find(")", ref_start);
            std::string ref = nets_section.substr(ref_start, ref_end - ref_start);
            size_t pin_pos = nets_section.find("(pin ", ref_end);
            if (pin_pos == std::string::npos) break;
            size_t pin_start = pin_pos + 5;
            size_t pin_end = nets_section.find(")", pin_start);
            std::string pin = nets_section.substr(pin_start, pin_end - pin_start);

            // Use connector nodes to classify signal direction
            if (ref.rfind("JIN_", 0) == 0) {
                auto it = signal_map.find(net_name);
                if (it != signal_map.end()) {
                    it->second->is_input = true;
                    if (it->second->is_output) {
                        it->second->is_internal = false;
                    }
                }
            } else if (ref.rfind("JOUT_", 0) == 0) {
                auto it = signal_map.find(net_name);
                if (it != signal_map.end()) {
                    it->second->is_output = true;
                    if (it->second->is_input) {
                        it->second->is_internal = false;
                    }
                }
            }

            // Connect only if component exists; otherwise treat as external connector
            if (component_map.find(ref) != component_map.end()) {
                connectSignal(ref, pin, net_name);
            }

            // advance
            node_pos = pin_end;
            // Stop this net's nodes when we hit a new (net
            size_t peek_net = nets_section.find("(net ", pin_end);
            size_t peek_node = nets_section.find("(node (ref ", pin_end);
            if (peek_net != std::string::npos && (peek_node == std::string::npos || peek_net < peek_node)) break;
        }
        net_pos = nets_section.find("(net ", net_pos + 5);
    }

    // Create explicit power signals if present
    if (signal_map.find("VCC") == signal_map.end()) createSignal("VCC", false, false);
    if (signal_map.find("GND") == signal_map.end()) createSignal("GND", false, false);

    module_name = "kicad_netlist";
    return true;
}

bool FModel::addComponent(const std::string& instance_id, const std::string& part_number, 
                         const std::string& package) {
    auto component = std::make_shared<ComponentInstance>(instance_id, part_number, package);
    component->component = createComponent(part_number);
    
    if (!component->component) {
        std::cerr << "Failed to create component: " << part_number << std::endl;
        return false;
    }
    
    components.push_back(component);
    component_map[instance_id] = component;
    
    std::cout << "Added component: " << instance_id << " (" << part_number << ")" << std::endl;
    return true;
}

std::shared_ptr<Component> FModel::createComponent(const std::string& part_number) {
    auto factory = component_factories.find(part_number);
    if (factory != component_factories.end()) {
        return factory->second();
    }
    
    std::cerr << "Unknown component type: " << part_number << std::endl;
    return nullptr;
}

bool FModel::connectSignal(const std::string& instance_id, const std::string& pin, 
                          const std::string& signal_name) {
    auto component = component_map.find(instance_id);
    if (component == component_map.end()) {
        std::cerr << "Component not found: " << instance_id << std::endl;
        return false;
    }
    
    component->second->addPinAssignment(pin, signal_name);
    
    // Create signal if it doesn't exist
    if (signal_map.find(signal_name) == signal_map.end()) {
        createSignal(signal_name, false, false);
    }
    
    std::cout << "Connected " << instance_id << " pin " << pin << " to signal " << signal_name << std::endl;
    return true;
}

std::shared_ptr<Signal> FModel::getSignal(const std::string& name) {
    auto it = signal_map.find(name);
    if (it != signal_map.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<Signal> FModel::createSignal(const std::string& name, bool is_input, bool is_output) {
    auto signal = std::make_shared<Signal>(name, is_input, is_output);
    signals.push_back(signal);
    signal_map[name] = signal;
    return signal;
}

void FModel::setSignalLevel(const std::string& signal_name, LogicLevel level) {
    auto signal = getSignal(signal_name);
    if (signal) {
        signal->setLevel(level);
    }
}

LogicLevel FModel::getSignalLevel(const std::string& signal_name) const {
    auto it = signal_map.find(signal_name);
    if (it != signal_map.end()) {
        return it->second->getLevel();
    }
    return LogicLevel::FLOATING;
}

bool FModel::loadTestVectors(const std::string& test_file) {
    std::cout << "Loading test vectors from: " << test_file << std::endl;
    
    if (!parseTestVectorFile(test_file)) {
        std::cerr << "Failed to parse test vector file: " << test_file << std::endl;
        return false;
    }
    
    std::cout << "Loaded " << test_vectors.size() << " test vectors" << std::endl;
    return true;
}

bool FModel::parseTestVectorFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open test vector file: " << filename << std::endl;
        return false;
    }
    
    std::string line;
    TestVector current_test;
    bool in_test = false;
    
    while (std::getline(file, line)) {
        // Remove leading/trailing whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        if (line.empty() || line[0] == '#') {
            continue; // Skip empty lines and comments
        }
        
        if (line[0] == '[' && line.back() == ']') {
            // New test vector
            if (in_test) {
                test_vectors.push_back(current_test);
            }
            current_test = TestVector(line.substr(1, line.length() - 2));
            in_test = true;
        } else if (in_test && line.find("=") != std::string::npos) {
            // Parse signal assignment
            size_t eq_pos = line.find("=");
            std::string signal_name = line.substr(0, eq_pos);
            std::string value_str = line.substr(eq_pos + 1);
            
            // Remove whitespace
            signal_name.erase(0, signal_name.find_first_not_of(" \t"));
            signal_name.erase(signal_name.find_last_not_of(" \t") + 1);
            value_str.erase(0, value_str.find_first_not_of(" \t"));
            value_str.erase(value_str.find_last_not_of(" \t") + 1);
            
            LogicLevel level = stringToLogicLevel(value_str);

            // Prefer direction info from netlist (JIN_/JOUT_ connectors)
            bool is_input = false;
            bool is_output = false;
            auto sig_it = signal_map.find(signal_name);
            if (sig_it != signal_map.end()) {
                is_input = sig_it->second->is_input;
                is_output = sig_it->second->is_output;
            }
            // Fallback to legacy heuristics only if unknown
            if (!is_input && !is_output) {
                if (signal_name.find("_in") != std::string::npos) {
                    is_input = true;
                }
                if (signal_name == "a" || signal_name == "b" || signal_name == "cin") {
                    is_input = true;
                }
                if (signal_name.rfind("a_", 0) == 0 || signal_name.rfind("b_", 0) == 0) {
                    is_input = true;
                }
                if (signal_name == "cout" || signal_name == "sum") {
                    is_output = true;
                }
                if (signal_name.rfind("sum_", 0) == 0) {
                    is_output = true;
                }
                if (signal_name == "sel" || (signal_name.size() >= 4 && signal_name.compare(signal_name.size()-4, 4, "_sel") == 0)) {
                    is_input = true;
                }
                if (signal_name == "y" || signal_name == "out" || (signal_name.size() >= 4 && signal_name.compare(signal_name.size()-4, 4, "_out") == 0)) {
                    is_output = true;
                }
            }

            if (is_input) {
                current_test.addInput(signal_name, level);
            } else if (is_output) {
                current_test.addExpectedOutput(signal_name, level);
            }
        }
    }
    
    if (in_test) {
        test_vectors.push_back(current_test);
    }
    
    file.close();
    return true;
}

void FModel::addTestVector(const TestVector& test_vector) {
    test_vectors.push_back(test_vector);
}

void FModel::clearTestVectors() {
    test_vectors.clear();
}

bool FModel::simulate() {
    if (!simulation_ready) {
        std::cerr << "Circuit not ready for simulation!" << std::endl;
        return false;
    }
    
    std::cout << "\n=== Starting Simulation ===" << std::endl;
    std::cout << "Running " << test_vectors.size() << " test vectors..." << std::endl;
    
    bool all_passed = true;
    
    for (size_t i = 0; i < test_vectors.size(); i++) {
        std::cout << "\n--- Test Vector " << (i + 1) << ": " << test_vectors[i].description << " ---" << std::endl;
        
        if (!simulateTestVector(test_vectors[i])) {
            all_passed = false;
        }
    }
    
    std::cout << "\n=== Simulation Complete ===" << std::endl;
    std::cout << "Overall Result: " << (all_passed ? "PASS" : "FAIL") << std::endl;
    
    return all_passed;
}

bool FModel::simulateTestVector(const TestVector& test_vector) {
    // Reset circuit
    resetCircuit();
    
    // Apply input stimuli
    for (const auto& input : test_vector.inputs) {
        setSignalLevel(input.first, input.second);
        std::cout << "Input " << input.first << " = " << logicLevelToString(input.second) << std::endl;
    }
    
    // Propagate signals through circuit generically
    propagateSignals();
    
    // Check outputs
    bool test_passed = true;
    std::cout << "\nOutputs:" << std::endl;
    
    for (const auto& expected : test_vector.expected_outputs) {
        LogicLevel actual = getSignalLevel(expected.first);
        LogicLevel expected_level = expected.second;
        
        std::cout << expected.first << ": Expected " << logicLevelToString(expected_level) 
                  << ", Got " << logicLevelToString(actual);
        
        if (actual == expected_level) {
            std::cout << " [PASS]" << std::endl;
        } else {
            std::cout << " [FAIL]" << std::endl;
            test_passed = false;
        }
    }
    
    return test_passed;
}

void FModel::propagateSignals() {
    // Iterate until stable or max iterations to propagate through multi-stage logic
    const int max_iters = 8;
    for (int iter = 0; iter < max_iters; ++iter) {
        // snapshot
        std::vector<LogicLevel> before;
        before.reserve(signals.size());
        for (const auto& s : signals) before.push_back(s->getLevel());

        updateComponentOutputs();

        // Removed hardcoded adder heuristics to keep generic across designs

        bool changed = false;
        for (size_t i = 0; i < signals.size(); ++i) {
            if (signals[i]->getLevel() != before[i]) { changed = true; break; }
        }
        if (!changed) break;
    }
}

void FModel::updateComponentOutputs() {
    auto toComponentLevel = [](LogicLevel lvl) -> Component::LogicLevel {
        switch (lvl) {
            case LogicLevel::LOW: return Component::LOW;
            case LogicLevel::HIGH: return Component::HIGH;
            case LogicLevel::FLOATING: default: return Component::FLOATING;
        }
    };
    auto toFmodelLevel = [](Component::LogicLevel lvl) -> LogicLevel {
        switch (lvl) {
            case Component::LOW: return LogicLevel::LOW;
            case Component::HIGH: return LogicLevel::HIGH;
            case Component::FLOATING: default: return LogicLevel::FLOATING;
        }
    };
    auto isPowerSignal = [](const std::string& name) {
        return name == "VCC" || name == "GND";
    };
    auto isOutputPin = [](const std::string& part, int pin) {
        if (part == "74HC02") {
            return pin == 1 || pin == 4 || pin == 10 || pin == 13;
        }
        if (part == "74HC04") {
            return pin == 2 || pin == 4 || pin == 6 || pin == 8 || pin == 10 || pin == 12;
        }
        if (part == "74HC74") {
            return pin == 5 || pin == 9; // Q outputs
        }
        // 74HC00/08/32/86 share outputs on 3,6,8,11
        return pin == 3 || pin == 6 || pin == 8 || pin == 11;
    };

    for (auto& compInst : components) {
        if (!compInst->component) continue;
        const std::string& part = compInst->part_number;

        // No special-cases: treat all components by declared pin roles

        // Drive inputs (generic)
        for (const auto& pa : compInst->pin_assignments) {
            int pinNum = std::stoi(pa.first);
            const std::string& sig = pa.second;
            if (isPowerSignal(sig)) continue;
            if (isOutputPin(part, pinNum)) continue; // don't drive outputs
            LogicLevel lvl = getSignalLevel(sig);
            compInst->component->setPin(pinNum, toComponentLevel(lvl));
        }

        // Read outputs (generic)
        for (const auto& pa : compInst->pin_assignments) {
            int pinNum = std::stoi(pa.first);
            const std::string& sig = pa.second;
            if (isPowerSignal(sig)) continue;
            if (!isOutputPin(part, pinNum)) continue; // only outputs
            Component::LogicLevel out = compInst->component->getPin(pinNum);
            if (out != Component::FLOATING) {
                setSignalLevel(sig, toFmodelLevel(out));
            }
        }
    }
}

void FModel::resetCircuit() {
    for (auto& signal : signals) {
        signal->setLevel(LogicLevel::FLOATING);
    }
    // Force power rails
    auto vcc_it = signal_map.find("VCC");
    if (vcc_it != signal_map.end()) vcc_it->second->setLevel(LogicLevel::HIGH);
    auto gnd_it = signal_map.find("GND");
    if (gnd_it != signal_map.end()) gnd_it->second->setLevel(LogicLevel::LOW);
}

bool FModel::validateCircuit() const {
    // Basic validation - check that all components have valid part numbers
    for (const auto& component : components) {
        if (component_factories.find(component->part_number) == component_factories.end()) {
            std::cerr << "Unknown component type: " << component->part_number << std::endl;
            return false;
        }
    }
    
    return true;
}

void FModel::printCircuitInfo() const {
    std::cout << "\n=== Circuit Information ===" << std::endl;
    std::cout << "Module: " << module_name << std::endl;
    std::cout << "Signals: " << signals.size() << std::endl;
    std::cout << "Components: " << components.size() << std::endl;
    
    std::cout << "\nSignals:" << std::endl;
    for (const auto& signal : signals) {
        std::cout << "  " << signal->name << " (" 
                  << (signal->is_input ? "input" : signal->is_output ? "output" : "internal") << ")" << std::endl;
    }
    
    std::cout << "\nComponents:" << std::endl;
    for (const auto& component : components) {
        std::cout << "  " << component->instance_id << " (" << component->part_number << ")" << std::endl;
    }
}

void FModel::printCircuitState() const {
    std::cout << "\n=== Circuit State ===" << std::endl;
    for (const auto& signal : signals) {
        std::cout << signal->name << " = " << logicLevelToString(signal->getLevel()) << std::endl;
    }
}

std::string FModel::logicLevelToString(LogicLevel level) const {
    switch (level) {
        case LogicLevel::LOW: return "LOW (0)";
        case LogicLevel::HIGH: return "HIGH (1)";
        case LogicLevel::FLOATING: return "FLOATING (Z)";
        default: return "UNKNOWN";
    }
}

LogicLevel FModel::stringToLogicLevel(const std::string& str) const {
    if (str == "0" || str == "LOW" || str == "low") {
        return LogicLevel::LOW;
    } else if (str == "1" || str == "HIGH" || str == "high") {
        return LogicLevel::HIGH;
    } else {
        return LogicLevel::FLOATING;
    }
}

} // namespace FModel
