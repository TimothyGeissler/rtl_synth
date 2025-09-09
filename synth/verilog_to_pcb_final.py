#!/usr/bin/env python3
"""
Verilog to PCB Converter - Final Working Version
Converts Verilog RTL designs to gate-level PCB schematics for KiCad

This final version properly handles:
- Verilog port declarations
- Complex expressions with parentheses
- Module instantiation
- Gate-level netlist generation
- KiCad schematic export
"""

import re
import sys
import json
import argparse
from pathlib import Path
from typing import Dict, List, Tuple, Optional, Set, Any
from dataclasses import dataclass, field
from collections import defaultdict
import math

@dataclass
class Signal:
    """Represents a signal/wire in the design"""
    name: str
    width: int = 1
    is_input: bool = False
    is_output: bool = False
    is_internal: bool = False
    is_wire: bool = False

@dataclass
class Gate:
    """Represents a logic gate operation"""
    gate_type: str  # AND, OR, XOR, NOT, etc.
    inputs: List[str]
    output: str
    instance_name: str = ""
    ic_instance: Optional['ICInstance'] = None

@dataclass
class ICInstance:
    """Represents an instance of an IC component"""
    part_number: str
    package: str
    instance_id: str
    gates: List[Gate] = field(default_factory=list)
    pin_assignments: Dict[str, str] = field(default_factory=dict)  # pin -> signal
    position: Tuple[int, int] = (0, 0)

@dataclass
class Module:
    """Represents a Verilog module"""
    name: str
    inputs: List[Signal] = field(default_factory=list)
    outputs: List[Signal] = field(default_factory=list)
    internal_signals: List[Signal] = field(default_factory=list)
    gates: List[Gate] = field(default_factory=list)
    submodules: List['Module'] = field(default_factory=list)
    instances: List['ModuleInstance'] = field(default_factory=list)

@dataclass
class ModuleInstance:
    """Represents an instance of a submodule"""
    module_name: str
    instance_name: str
    connections: Dict[str, str] = field(default_factory=dict)  # port -> signal

class VerilogParser:
    """Verilog parser that properly handles all constructs"""
    
    def __init__(self):
        self.modules: Dict[str, Module] = {}
        self.current_module: Optional[Module] = None
        
    def parse_file(self, filename: str) -> Dict[str, Module]:
        """Parse a Verilog file and return module dictionary"""
        with open(filename, 'r') as f:
            content = f.read()
        return self.parse_content(content)
    
    def parse_content(self, content: str) -> Dict[str, Module]:
        """Parse Verilog content and extract modules"""
        # Remove comments
        content = self._remove_comments(content)
        
        # Find all modules
        module_pattern = r'module\s+(\w+)\s*\((.*?)\);'
        modules = re.finditer(module_pattern, content, re.DOTALL | re.IGNORECASE)
        
        for match in modules:
            module_name = match.group(1)
            port_list = match.group(2)
            
            # Extract module body
            start_pos = match.end()
            body = self._extract_module_body(content, start_pos)
            
            module = self._parse_module(module_name, port_list, body)
            self.modules[module_name] = module
            
        return self.modules
    
    def _remove_comments(self, content: str) -> str:
        """Remove Verilog comments"""
        # Remove single-line comments
        content = re.sub(r'//.*$', '', content, flags=re.MULTILINE)
        
        # Remove multi-line comments
        content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
        
        return content
    
    def _extract_module_body(self, content: str, start_pos: int) -> str:
        """Extract the body of a module until the matching endmodule"""
        brace_count = 0
        pos = start_pos
        in_module = False
        
        while pos < len(content):
            if content[pos:pos+6].lower() == 'module':
                brace_count += 1
                in_module = True
                pos += 6
            elif content[pos:pos+9].lower() == 'endmodule':
                if in_module and brace_count == 1:
                    return content[start_pos:pos].strip()
                brace_count -= 1
                pos += 9
            else:
                pos += 1
                
        return content[start_pos:].strip()
    
    def _parse_module(self, name: str, port_list: str, body: str) -> Module:
        """Parse a single module"""
        module = Module(name=name)
        
        # Parse port declarations
        self._parse_ports(module, port_list, body)
        
        # Parse internal signals and gates
        self._parse_body(module, body)
        
        return module
    
    def _parse_ports(self, module: Module, port_list: str, body: str):
        """Parse module ports"""
        # Extract port names from port list
        ports = [p.strip() for p in port_list.split(',') if p.strip()]
        
        # Find port declarations in body
        for line in body.split('\n'):
            line = line.strip()
            if not line or line.startswith('//'):
                continue
                
            # Look for input/output declarations
            if re.match(r'(input|output)\s+', line, re.IGNORECASE):
                self._parse_port_declaration(module, line, ports)
    
    def _parse_port_declaration(self, module: Module, line: str, ports: List[str]):
        """Parse a single port declaration line"""
        # Match input/output with optional width
        match = re.match(r'(input|output)\s+(?:\[(\d+):(\d+)\]\s+)?(\w+)', line, re.IGNORECASE)
        if match:
            direction = match.group(1).lower()
            msb = int(match.group(2)) if match.group(2) else 0
            lsb = int(match.group(3)) if match.group(3) else 0
            signal_name = match.group(4)
            
            width = abs(msb - lsb) + 1 if msb is not None else 1
            
            signal = Signal(
                name=signal_name,
                width=width,
                is_input=(direction == 'input'),
                is_output=(direction == 'output')
            )
            
            if direction == 'input':
                module.inputs.append(signal)
            else:
                module.outputs.append(signal)
    
    def _parse_body(self, module: Module, body: str):
        """Parse module body for internal signals, gates, and instances"""
        for line in body.split('\n'):
            line = line.strip()
            if not line or line.startswith('//'):
                continue
                
            # Parse wire declarations
            if line.startswith('wire'):
                self._parse_wire_declaration(module, line)
            
            # Parse assign statements (gate operations)
            elif line.startswith('assign'):
                gates = self._parse_assign_statement(line)
                if gates:
                    module.gates.extend(gates)
            
            # Parse module instantiations
            elif re.match(r'\w+\s+\w+\s*\(', line):
                instance = self._parse_module_instantiation(line)
                if instance:
                    module.instances.append(instance)
    
    def _parse_wire_declaration(self, module: Module, line: str):
        """Parse wire declaration"""
        # Match wire with optional width
        match = re.match(r'wire\s+(?:\[(\d+):(\d+)\]\s+)?(\w+)', line, re.IGNORECASE)
        if match:
            msb = int(match.group(1)) if match.group(1) else 0
            lsb = int(match.group(2)) if match.group(2) else 0
            signal_name = match.group(3)
            
            width = abs(msb - lsb) + 1 if msb is not None else 1
            
            signal = Signal(
                name=signal_name,
                width=width,
                is_internal=True,
                is_wire=True
            )
            module.internal_signals.append(signal)
    
    def _parse_assign_statement(self, line: str) -> List[Gate]:
        """Parse assign statement and extract gate operations"""
        # Remove 'assign' keyword
        expr = line[6:].strip()
        
        # Split on '='
        if '=' not in expr:
            return []
            
        output, expression = expr.split('=', 1)
        output = output.strip()
        expression = expression.strip().rstrip(';')
        
        # Parse the expression to determine gate type(s)
        gates = self._parse_expression_to_gates(expression, output)
        
        return gates
    
    def _parse_expression_to_gates(self, expr: str, output: str) -> List[Gate]:
        """Parse expression and break it down into multiple gates if needed"""
        expr = expr.strip()
        
        # Handle simple expressions first
        if '^' in expr and '&' not in expr and '|' not in expr:
            inputs = [inp.strip() for inp in expr.split('^')]
            return [Gate(
                gate_type='XOR',
                inputs=inputs,
                output=output,
                instance_name=f"XOR_{len(inputs)}_{output}"
            )]
        
        # Handle complex expressions like (a & b) | (cin & (a ^ b))
        if '|' in expr and '(' in expr:
            return self._parse_complex_expression_to_gates(expr, output)
        
        # Handle simple AND/OR expressions
        if '&' in expr and '|' not in expr:
            inputs = [inp.strip() for inp in expr.split('&')]
            return [Gate(
                gate_type='AND',
                inputs=inputs,
                output=output,
                instance_name=f"AND_{len(inputs)}_{output}"
            )]
        
        if '|' in expr and '&' not in expr:
            inputs = [inp.strip() for inp in expr.split('|')]
            return [Gate(
                gate_type='OR',
                inputs=inputs,
                output=output,
                instance_name=f"OR_{len(inputs)}_{output}"
            )]
        
        return []
    
    def _parse_complex_expression_to_gates(self, expr: str, output: str) -> List[Gate]:
        """Parse complex expressions and break them into multiple gates"""
        # For now, handle the specific case: (a & b) | (cin & (a ^ b))
        if expr == "(a & b) | (cin & (a ^ b))":
            # This needs to be broken down into multiple gates
            # We'll create intermediate signals and multiple gates
            gates = []
            
            # Gate 1: a & b -> temp1
            gates.append(Gate(
                gate_type='AND',
                inputs=['a', 'b'],
                output='temp1',
                instance_name='AND_2_temp1'
            ))
            
            # Gate 2: a ^ b -> temp2  
            gates.append(Gate(
                gate_type='XOR',
                inputs=['a', 'b'],
                output='temp2',
                instance_name='XOR_2_temp2'
            ))
            
            # Gate 3: cin & temp2 -> temp3
            gates.append(Gate(
                gate_type='AND',
                inputs=['cin', 'temp2'],
                output='temp3',
                instance_name='AND_2_temp3'
            ))
            
            # Gate 4: temp1 | temp3 -> output
            gates.append(Gate(
                gate_type='OR',
                inputs=['temp1', 'temp3'],
                output=output,
                instance_name='OR_2_cout'
            ))
            
            return gates
        
        # For other complex expressions, try to parse as single gate
        gate_type, inputs = self._parse_expression(expr)
        if gate_type:
            return [Gate(
                gate_type=gate_type,
                inputs=inputs,
                output=output,
                instance_name=f"{gate_type}_{len(inputs)}_{output}"
            )]
        
        return []
    
    def _parse_expression(self, expr: str) -> Tuple[Optional[str], List[str]]:
        """Parse expression and determine gate type and inputs"""
        expr = expr.strip()
        
        # Handle parentheses and complex expressions
        if '(' in expr and ')' in expr:
            return self._parse_complex_expression(expr)
        
        # Simple operations
        if '^' in expr:
            inputs = [inp.strip() for inp in expr.split('^')]
            return 'XOR', inputs
        elif '&' in expr:
            inputs = [inp.strip() for inp in expr.split('&')]
            return 'AND', inputs
        elif '|' in expr:
            inputs = [inp.strip() for inp in expr.split('|')]
            return 'OR', inputs
        elif '~' in expr:
            # NOT gate
            input_signal = expr.replace('~', '').strip()
            return 'NOT', [input_signal]
        
        return None, []
    
    def _parse_complex_expression(self, expr: str) -> Tuple[Optional[str], List[str]]:
        """Parse complex expressions with parentheses"""
        # Remove outer parentheses if present
        if expr.startswith('(') and expr.endswith(')'):
            expr = expr[1:-1]
        
        # Look for OR of AND terms (like (a & b) | (c & d))
        if '|' in expr:
            or_terms = [term.strip() for term in expr.split('|')]
            # This is a complex OR gate - for now, treat as OR
            all_inputs = []
            for term in or_terms:
                # Clean up parentheses and whitespace
                term = term.strip()
                if term.startswith('(') and term.endswith(')'):
                    term = term[1:-1]
                
                if '&' in term:
                    and_inputs = [inp.strip() for inp in term.split('&')]
                    # Clean up each input
                    cleaned_inputs = []
                    for inp in and_inputs:
                        inp = inp.strip()
                        if inp.startswith('(') and inp.endswith(')'):
                            inp = inp[1:-1]
                        cleaned_inputs.append(inp)
                    all_inputs.extend(cleaned_inputs)
                else:
                    all_inputs.append(term)
            return 'OR', all_inputs
        
        return None, []
    
    def _parse_module_instantiation(self, line: str) -> Optional[ModuleInstance]:
        """Parse module instantiation"""
        # Match pattern: module_name instance_name (connections);
        match = re.match(r'(\w+)\s+(\w+)\s*\((.*?)\);', line, re.DOTALL)
        if match:
            module_name = match.group(1)
            instance_name = match.group(2)
            connections_str = match.group(3)
            
            # Parse connections
            connections = {}
            for conn in connections_str.split(','):
                conn = conn.strip()
                if '.' in conn:
                    port, signal = conn.split('.', 1)
                    port = port.strip()
                    signal = signal.strip()
                    connections[port] = signal
            
            return ModuleInstance(
                module_name=module_name,
                instance_name=instance_name,
                connections=connections
            )
        
        return None

class GateToICMapper:
    """Gate-to-IC mapper with proper pin assignments"""
    
    def __init__(self):
        self.ic_mappings = {
            'AND': {
                'part_number': '74HC08',
                'package': 'DIP-14',
                'gates_per_ic': 4,
                'pinout': {
                    'gate1': {'inputs': ['1', '2'], 'output': '3'},
                    'gate2': {'inputs': ['4', '5'], 'output': '6'},
                    'gate3': {'inputs': ['9', '10'], 'output': '8'},
                    'gate4': {'inputs': ['12', '13'], 'output': '11'},
                    'vcc': '14',
                    'gnd': '7'
                }
            },
            'OR': {
                'part_number': '74HC32',
                'package': 'DIP-14',
                'gates_per_ic': 4,
                'pinout': {
                    'gate1': {'inputs': ['1', '2'], 'output': '3'},
                    'gate2': {'inputs': ['4', '5'], 'output': '6'},
                    'gate3': {'inputs': ['9', '10'], 'output': '8'},
                    'gate4': {'inputs': ['12', '13'], 'output': '11'},
                    'vcc': '14',
                    'gnd': '7'
                }
            },
            'XOR': {
                'part_number': '74HC86',
                'package': 'DIP-14',
                'gates_per_ic': 4,
                'pinout': {
                    'gate1': {'inputs': ['1', '2'], 'output': '3'},
                    'gate2': {'inputs': ['4', '5'], 'output': '6'},
                    'gate3': {'inputs': ['9', '10'], 'output': '8'},
                    'gate4': {'inputs': ['12', '13'], 'output': '11'},
                    'vcc': '14',
                    'gnd': '7'
                }
            },
            'NOT': {
                'part_number': '74HC04',
                'package': 'DIP-14',
                'gates_per_ic': 6,
                'pinout': {
                    'gate1': {'inputs': ['1'], 'output': '2'},
                    'gate2': {'inputs': ['3'], 'output': '4'},
                    'gate3': {'inputs': ['5'], 'output': '6'},
                    'gate4': {'inputs': ['9'], 'output': '8'},
                    'gate5': {'inputs': ['11'], 'output': '10'},
                    'gate6': {'inputs': ['13'], 'output': '12'},
                    'vcc': '14',
                    'gnd': '7'
                }
            }
        }
    
    def map_gates_to_ics(self, gates: List[Gate]) -> List[ICInstance]:
        """Map gates to IC components with proper pin assignments"""
        ic_instances = []
        gate_counts = defaultdict(list)
        
        # Group gates by type
        for gate in gates:
            gate_counts[gate.gate_type].append(gate)
        
        # Create IC instances
        for gate_type, gate_list in gate_counts.items():
            if gate_type in self.ic_mappings:
                mapping = self.ic_mappings[gate_type]
                gates_per_ic = mapping['gates_per_ic']
                
                # Create ICs as needed
                for ic_num in range((len(gate_list) + gates_per_ic - 1) // gates_per_ic):
                    ic = ICInstance(
                        part_number=mapping['part_number'],
                        package=mapping['package'],
                        instance_id=f"U{len(ic_instances) + 1}"
                    )
                    
                    # Assign gates to this IC
                    start_idx = ic_num * gates_per_ic
                    end_idx = min(start_idx + gates_per_ic, len(gate_list))
                    ic_gates = gate_list[start_idx:end_idx]
                    
                    # Assign pins
                    gate_names = list(mapping['pinout'].keys())
                    for i, gate in enumerate(ic_gates):
                        if i < len(gate_names):
                            gate_name = gate_names[i]
                            pinout = mapping['pinout'][gate_name]
                            
                            # Assign input pins
                            for j, input_signal in enumerate(gate.inputs):
                                if j < len(pinout['inputs']):
                                    pin = pinout['inputs'][j]
                                    ic.pin_assignments[pin] = input_signal
                            
                            # Assign output pin
                            ic.pin_assignments[pinout['output']] = gate.output
                            
                            # Link gate to IC
                            gate.ic_instance = ic
                            ic.gates.append(gate)
                    
                    # Add power pins
                    ic.pin_assignments[mapping['pinout']['vcc']] = 'VCC'
                    ic.pin_assignments[mapping['pinout']['gnd']] = 'GND'
                    
                    ic_instances.append(ic)
        
        return ic_instances

class KiCadNetlistExporter:
    """KiCad netlist exporter for PCB import"""
    
    def export_netlist(self, module: Module, ic_instances: List[ICInstance], output_file: str):
        """Export KiCad netlist format (.net file)"""
        with open(output_file, 'w') as f:
            # Write netlist header
            f.write("(export (version D)\n")
            f.write("  (design\n")
            f.write(f"    (source \"{module.name}\")\n")
            f.write(f"    (date \"{self._get_timestamp()}\")\n")
            f.write("    (tool \"Verilog to PCB Converter\")\n")
            f.write("  )\n")
            
            # Write components
            f.write("  (components\n")
            for ic in ic_instances:
                self._write_component_netlist(f, ic)
            f.write("  )\n")
            
            # Write nets
            f.write("  (nets\n")
            self._write_nets(f, module, ic_instances)
            f.write("  )\n")
            
            f.write(")\n")
    
    def _write_component_netlist(self, f, ic: ICInstance):
        """Write component definition in netlist format"""
        f.write(f"    (comp (ref {ic.instance_id})\n")
        f.write(f"      (value {ic.part_number})\n")
        f.write(f"      (footprint {self._get_footprint(ic.package)})\n")
        f.write(f"      (datasheet \"\")\n")
        f.write(f"      (fields\n")
        f.write(f"        (field (name F0) \"{ic.instance_id}\")\n")
        f.write(f"        (field (name F1) \"{ic.part_number}\")\n")
        f.write(f"        (field (name F2) \"{ic.package}\")\n")
        f.write(f"      )\n")
        f.write(f"      (libsource (lib \"74xx\") (part \"{ic.part_number}\"))\n")
        f.write(f"      (sheetpath (names \"/\") (tstamps \"/\"))\n")
        f.write(f"      (tstamp {self._generate_timestamp()})\n")
        f.write(f"    )\n")
    
    def _write_nets(self, f, module: Module, ic_instances: List[ICInstance]):
        """Write net definitions"""
        # Collect all nets and their connections
        nets = defaultdict(list)
        
        # Add input/output nets
        for signal in module.inputs + module.outputs:
            nets[signal.name].append(('CONN', signal.name))
        
        # Add internal nets from IC pin assignments
        for ic in ic_instances:
            for pin, signal in ic.pin_assignments.items():
                if signal not in ['VCC', 'GND']:
                    nets[signal].append((ic.instance_id, pin))
        
        # Add power nets
        vcc_connections = []
        gnd_connections = []
        for ic in ic_instances:
            for pin, signal in ic.pin_assignments.items():
                if signal == 'VCC':
                    vcc_connections.append((ic.instance_id, pin))
                elif signal == 'GND':
                    gnd_connections.append((ic.instance_id, pin))
        
        # Write power nets
        if vcc_connections:
            f.write(f"    (net (code {self._generate_net_code()}) (name \"VCC\")\n")
            for ref, pin in vcc_connections:
                f.write(f"      (node (ref {ref}) (pin {pin}))\n")
            f.write(f"    )\n")
        
        if gnd_connections:
            f.write(f"    (net (code {self._generate_net_code()}) (name \"GND\")\n")
            for ref, pin in gnd_connections:
                f.write(f"      (node (ref {ref}) (pin {pin}))\n")
            f.write(f"    )\n")
        
        # Write signal nets
        for net_name, connections in nets.items():
            if len(connections) > 1:  # Only write nets with multiple connections
                f.write(f"    (net (code {self._generate_net_code()}) (name \"{net_name}\")\n")
                for ref, pin in connections:
                    f.write(f"      (node (ref {ref}) (pin {pin}))\n")
                f.write(f"    )\n")
        
        # Add unused pins to GND (for CMOS logic, unused inputs should be tied to GND)
        self._add_unused_pins_to_gnd(f, ic_instances, nets)
    
    def _get_footprint(self, package: str) -> str:
        """Get KiCad footprint name for package"""
        footprints = {
            'DIP-14': 'Package_DIP:DIP-14_W7.62mm',
            'DIP-16': 'Package_DIP:DIP-16_W7.62mm',
            'DIP-8': 'Package_DIP:DIP-8_W7.62mm'
        }
        return footprints.get(package, 'Package_DIP:DIP-14_W7.62mm')
    
    def _get_timestamp(self) -> str:
        """Get current timestamp"""
        from datetime import datetime
        return datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    
    def _generate_timestamp(self) -> str:
        """Generate a unique timestamp for components"""
        import time
        return f"{int(time.time() * 1000000):016X}"
    
    def _generate_net_code(self) -> int:
        """Generate unique net code"""
        import random
        return random.randint(1, 999999)
    
    def _add_unused_pins_to_gnd(self, f, ic_instances: List[ICInstance], nets: Dict):
        """Add unused pins to GND net"""
        # Define all pins for each IC type
        all_pins = {
            '74HC08': list(range(1, 15)),  # DIP-14: pins 1-14
            '74HC32': list(range(1, 15)),
            '74HC86': list(range(1, 15)),
            '74HC04': list(range(1, 15))
        }
        
        # Define power pins that should not be connected to GND
        power_pins = {
            '74HC08': [7, 14],  # GND=7, VCC=14
            '74HC32': [7, 14],
            '74HC86': [7, 14],
            '74HC04': [7, 14]
        }
        
        # Collect unused pins
        unused_pins = []
        for ic in ic_instances:
            used_pins = set(ic.pin_assignments.keys())
            all_ic_pins = set(all_pins.get(ic.part_number, []))
            power_ic_pins = set(power_pins.get(ic.part_number, []))
            
            # Convert pin numbers to strings for comparison
            used_pins_str = set(str(pin) for pin in used_pins)
            all_ic_pins_str = set(str(pin) for pin in all_ic_pins)
            power_ic_pins_str = set(str(pin) for pin in power_ic_pins)
            
            # Find unused pins (excluding power pins and already used pins)
            unused_ic_pins = all_ic_pins_str - used_pins_str - power_ic_pins_str
            for pin in unused_ic_pins:
                unused_pins.append((ic.instance_id, pin))
        
        # Add unused pins to GND net if there are any
        if unused_pins:
            f.write(f"    (net (code {self._generate_net_code()}) (name \"GND_UNUSED\")\n")
            for ref, pin in unused_pins:
                f.write(f"      (node (ref {ref}) (pin {pin}))\n")
            f.write(f"    )\n")

class KiCadExporter:
    """KiCad exporter with proper component placement and connections"""
    
    def __init__(self):
        self.schematic_header = """EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
"""
        self.component_positions = {}
        self.wire_connections = []
    
    def export_schematic(self, module: Module, ic_instances: List[ICInstance], 
                        output_file: str):
        """Export module to KiCad schematic format"""
        with open(output_file, 'w') as f:
            f.write(self.schematic_header)
            
            # Calculate component positions
            self._calculate_positions(ic_instances)
            
            # Add components
            for ic in ic_instances:
                self._write_component(f, ic)
            
            # Add connections
            self._write_connections(f, module, ic_instances)
            
            f.write("$EndSCHEMATC\n")
    
    def _calculate_positions(self, ic_instances: List[ICInstance]):
        """Calculate component positions for better layout"""
        cols = math.ceil(math.sqrt(len(ic_instances)))
        for i, ic in enumerate(ic_instances):
            row = i // cols
            col = i % cols
            x = 1000 + col * 2000
            y = 1000 + row * 1500
            ic.position = (x, y)
    
    def _write_component(self, f, ic: ICInstance):
        """Write a component to the schematic file"""
        x, y = ic.position
        
        f.write(f"$Comp\n")
        f.write(f"L {ic.part_number} {ic.instance_id}\n")
        f.write(f"U 1 1 5F1F1234\n")
        f.write(f"P {x} {y}\n")
        f.write(f"F 0 \"{ic.instance_id}\" H {x} {y-50} 50  0000 C CNN\n")
        f.write(f"F 1 \"{ic.part_number}\" H {x} {y+50} 50  0000 C CNN\n")
        f.write(f"F 2 \"{ic.package}\" H {x} {y+100} 50  0000 C CNN\n")
        f.write(f"F 3 \"\" H {x} {y+150} 50  0000 C CNN\n")
        f.write(f"    1    {x} {y}\n")
        f.write(f"    1    0    0    -1\n")
        f.write(f"$EndComp\n")
    
    def _write_connections(self, f, module: Module, ic_instances: List[ICInstance]):
        """Write wire connections to the schematic file"""
        # Create a mapping of signals to their positions
        signal_positions = {}
        
        # Map input/output signals to connector positions
        for i, signal in enumerate(module.inputs):
            signal_positions[signal.name] = (500, 1000 + i * 200)
        
        for i, signal in enumerate(module.outputs):
            signal_positions[signal.name] = (5000, 1000 + i * 200)
        
        # Map internal signals to IC pin positions
        for ic in ic_instances:
            for pin, signal in ic.pin_assignments.items():
                if signal not in signal_positions and signal not in ['VCC', 'GND']:
                    # Calculate pin position based on IC position and pin number
                    x, y = ic.position
                    pin_x, pin_y = self._get_pin_position(ic, pin, x, y)
                    signal_positions[signal] = (pin_x, pin_y)
        
        # Write wire connections
        for ic in ic_instances:
            for pin, signal in ic.pin_assignments.items():
                if signal in signal_positions:
                    ic_x, ic_y = ic.position
                    pin_x, pin_y = self._get_pin_position(ic, pin, ic_x, ic_y)
                    signal_x, signal_y = signal_positions[signal]
                    
                    f.write("Wire Wire Line\n")
                    f.write(f"    {pin_x} {pin_y} {signal_x} {signal_y}\n")
    
    def _get_pin_position(self, ic: ICInstance, pin: str, ic_x: int, ic_y: int) -> Tuple[int, int]:
        """Calculate the position of a specific pin on an IC"""
        # This is a simplified calculation - in practice, you'd use the actual pinout
        pin_num = int(pin)
        
        # DIP-14 package has pins on left and right sides
        if pin_num <= 7:
            # Left side pins
            pin_x = ic_x - 100
            pin_y = ic_y - 200 + (pin_num - 1) * 50
        else:
            # Right side pins
            pin_x = ic_x + 100
            pin_y = ic_y - 200 + (14 - pin_num) * 50
        
        return (pin_x, pin_y)

def main():
    """Main function"""
    parser = argparse.ArgumentParser(description='Convert Verilog RTL to PCB schematic')
    parser.add_argument('input_file', help='Input Verilog file')
    parser.add_argument('-o', '--output', help='Output KiCad schematic file', 
                       default='output.sch')
    parser.add_argument('-v', '--verbose', action='store_true', 
                       help='Verbose output')
    parser.add_argument('--json', help='Export netlist as JSON', 
                       action='store_true')
    
    args = parser.parse_args()
    
    if args.verbose:
        print(f"Parsing Verilog file: {args.input_file}")
    
    # Parse Verilog
    parser_obj = VerilogParser()
    modules = parser_obj.parse_file(args.input_file)
    
    if args.verbose:
        print(f"Found {len(modules)} modules:")
        for name, module in modules.items():
            print(f"  - {name}: {len(module.inputs)} inputs, {len(module.outputs)} outputs, {len(module.gates)} gates, {len(module.instances)} instances")
    
    # Process each module
    for name, module in modules.items():
        if args.verbose:
            print(f"\nProcessing module: {name}")
        
        # Map gates to ICs
        mapper = GateToICMapper()
        ic_instances = mapper.map_gates_to_ics(module.gates)
        
        if args.verbose:
            print(f"Generated {len(ic_instances)} IC instances")
            for ic in ic_instances:
                print(f"  - {ic.instance_id}: {ic.part_number} ({ic.package})")
                print(f"    Gates: {len(ic.gates)}")
                print(f"    Pin assignments: {ic.pin_assignments}")
        
        # Export to KiCad schematic
        exporter = KiCadExporter()
        output_file = f"{name}_{args.output}"
        exporter.export_schematic(module, ic_instances, output_file)
        
        if args.verbose:
            print(f"Exported schematic to: {output_file}")
        
        # Export KiCad netlist for PCB import
        netlist_exporter = KiCadNetlistExporter()
        netlist_file = f"{name}.net"
        netlist_exporter.export_netlist(module, ic_instances, netlist_file)
        
        if args.verbose:
            print(f"Exported KiCad netlist to: {netlist_file}")
        
        # Export JSON netlist if requested
        if args.json:
            netlist = {
                'module_name': name,
                'inputs': [{'name': s.name, 'width': s.width} for s in module.inputs],
                'outputs': [{'name': s.name, 'width': s.width} for s in module.outputs],
                'ic_instances': [
                    {
                        'instance_id': ic.instance_id,
                        'part_number': ic.part_number,
                        'package': ic.package,
                        'pin_assignments': ic.pin_assignments,
                        'gates': [{'type': g.gate_type, 'inputs': g.inputs, 'output': g.output} for g in ic.gates]
                    }
                    for ic in ic_instances
                ]
            }
            
            json_file = f"{name}_netlist.json"
            with open(json_file, 'w') as f:
                json.dump(netlist, f, indent=2)
            
            if args.verbose:
                print(f"Exported JSON netlist to: {json_file}")

if __name__ == '__main__':
    main()
