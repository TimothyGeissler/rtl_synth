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
        self._temp_counter: int = 0
        
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
        # Prefer ANSI-style ports declared in the header (port_list)
        header = port_list.replace('\n', ' ')
        # Match sequences like: input [3:0] a or output cout
        for m in re.finditer(r'(input|output)\s+(?:\[(\d+):(\d+)\]\s+)?(\w+)', header, re.IGNORECASE):
            direction = m.group(1).lower()
            msb = int(m.group(2)) if m.group(2) else 0
            lsb = int(m.group(3)) if m.group(3) else 0
            name = m.group(4)
            width = abs(msb - lsb) + 1 if m.group(2) else 1
            sig = Signal(name=name, width=width, is_input=(direction=='input'), is_output=(direction=='output'))
            if sig.is_input:
                module.inputs.append(sig)
            else:
                module.outputs.append(sig)
        
        # Also support non-ANSI style (declared in body)
        for line in body.split('\n'):
            line = line.strip()
            if not line or line.startswith('//'):
                continue
            if re.match(r'(input|output)\s+', line, re.IGNORECASE):
                self._parse_port_declaration(module, line, [])
    
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
        lines = body.split('\n')
        i = 0
        while i < len(lines):
            line = lines[i].strip()
            if not line or line.startswith('//'):
                i += 1
                continue
                
            # Parse wire declarations
            if line.startswith('wire'):
                self._parse_wire_declaration(module, line)
                i += 1
                continue
            
            # Parse assign statements (gate operations)
            if line.startswith('assign'):
                gates = self._parse_assign_statement(line)
                if gates:
                    module.gates.extend(gates)
                i += 1
                continue

            # Parse simple DFF behavioral block: always @(posedge clk) q <= d;
            if line.startswith('always'):
                header = line
                block = ''
                # accumulate lines until 'end' for multi-line blocks
                if 'begin' in line and 'end' not in line:
                    i += 1
                    while i < len(lines):
                        l2 = lines[i].strip()
                        if l2.startswith('end'):
                            break
                        block += ' ' + l2
                        i += 1
                else:
                    block = line
                mclk = re.search(r'posedge\s+(\w+)', header)
                if mclk:
                    clk = mclk.group(1)
                    # Find all nonblocking assignments within the block
                    for m in re.finditer(r'(\w+)\s*<=\s*(\w+)\s*;?', block):
                        q = m.group(1)
                        d = m.group(2)
                        module.gates.append(Gate(gate_type='DFF', inputs=[sanitize_signal_name(d), sanitize_signal_name(clk)], output=sanitize_signal_name(q)))
                i += 1
                continue
            
            # Parse module instantiations
            if re.match(r'\w+\s+\w+\s*\(', line):
                # Capture multiline instantiation until closing ');'
                inst_text = line
                if not line.strip().endswith(');'):
                    i += 1
                    while i < len(lines):
                        inst_text += ' ' + lines[i].strip()
                        if inst_text.strip().endswith(');'):
                            break
                        i += 1
                instance = self._parse_module_instantiation(inst_text)
                if instance:
                    module.instances.append(instance)
                i += 1
                continue
            i += 1
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
        
        # Simple alias: output = identifier; -> record ALIAS (wire tie)
        if re.match(r'^\w+$', expression):
            return [Gate(gate_type='ALIAS', inputs=[expression], output=output)]

        # Parse the expression generically into gates
        gates = self._parse_boolean_expr_to_gates(expression, output)
        
        return gates
    
    def _parse_boolean_expr_to_gates(self, expr: str, output: str) -> List[Gate]:
        """Generic boolean expression parser to gates for ~, &, ^, | with parentheses.
        Uses shunting-yard to parse into RPN, builds an AST, then lowers to 74xx gates.
        """
        # First, rewrite any ternary operators to boolean primitives
        expr = self._rewrite_ternary_to_bool(expr)
        tokens = self._tokenize_boolean_expr(expr)
        rpn = self._shunting_yard(tokens)
        if not rpn:
            return []
        ast = self._rpn_to_ast(rpn)
        gates: List[Gate] = []
        _ = self._ast_to_gates(ast, output, gates)
        return gates

    def _rewrite_ternary_to_bool(self, expr: str) -> str:
        """Rewrite cond ? a : b into (cond & a) | (~cond & b), recursively for nested ternaries."""
        expr = expr.strip()
        # Quick exit
        if '?' not in expr:
            return expr
        # Find top-level '?'
        def find_top_ternary(s: str) -> Optional[Tuple[int, int, int]]:
            depth = 0
            q_pos = -1
            i = 0
            while i < len(s):
                c = s[i]
                if c == '(':
                    depth += 1
                elif c == ')':
                    depth -= 1 if depth > 0 else 0
                elif c == '?' and depth == 0 and q_pos == -1:
                    q_pos = i
                elif c == ':' and depth == 0 and q_pos != -1:
                    # Found matching ':' for this '?'
                    return (q_pos, i, 0)
                i += 1
            return None
        # Recursive rewrite
        def rewrite(s: str) -> str:
            res = find_top_ternary(s)
            if not res:
                return s
            q_idx, colon_idx, _ = res
            cond = s[:q_idx].strip()
            then_part = s[q_idx+1:colon_idx].strip()
            else_part = s[colon_idx+1:].strip()
            # Handle nested ternary in then/else via recursion
            cond_r = rewrite(cond)
            then_r = rewrite(then_part)
            else_r = rewrite(else_part)
            return f"(({cond_r}) & ({then_r})) | ((~({cond_r})) & ({else_r}))"
        return rewrite(expr)

    def _tokenize_boolean_expr(self, expr: str) -> List[str]:
        tokens: List[str] = []
        i = 0
        while i < len(expr):
            c = expr[i]
            if c.isspace():
                i += 1
                continue
            if c in '()~&^|':
                tokens.append(c)
                i += 1
                continue
            # identifier [a-zA-Z0-9_]
            j = i
            while j < len(expr) and (expr[j].isalnum() or expr[j] == '_'):
                j += 1
            tokens.append(expr[i:j])
            i = j
        return tokens

    def _shunting_yard(self, tokens: List[str]) -> List[str]:
        # Precedence: ~ > & > ^ > |
        prec = {'~': 4, '&': 3, '^': 2, '|': 1}
        right_assoc = {'~'}
        output: List[str] = []
        ops: List[str] = []
        i = 0
        while i < len(tokens):
            t = tokens[i]
            if re.match(r'^[A-Za-z_][A-Za-z0-9_]*$', t):
                output.append(t)
            elif t == '(':
                ops.append(t)
            elif t == ')':
                while ops and ops[-1] != '(':
                    output.append(ops.pop())
                if ops and ops[-1] == '(':
                    ops.pop()
            elif t in prec:
                # Handle unary ~ (could appear multiple times)
                while ops and ops[-1] in prec and (
                    (ops[-1] not in right_assoc and prec[ops[-1]] >= prec[t]) or
                    (ops[-1] in right_assoc and prec[ops[-1]] > prec[t])
                ):
                    output.append(ops.pop())
                ops.append(t)
            i += 1
        while ops:
            output.append(ops.pop())
        return output

    def _rpn_to_ast(self, rpn: List[str]):
        stack: List[Any] = []
        for t in rpn:
            if re.match(r'^[A-Za-z_][A-Za-z0-9_]*$', t):
                stack.append(('id', t))
            elif t == '~':
                a = stack.pop()
                stack.append(('not', a))
            elif t in ('&', '^', '|'):
                b = stack.pop(); a = stack.pop()
                op_map = {'&': 'and', '^': 'xor', '|': 'or'}
                stack.append((op_map[t], a, b))
        return stack[-1] if stack else None

    def _new_temp(self, base: str) -> str:
        self._temp_counter += 1
        return f"tmp_{base}_{self._temp_counter}"

    def _ast_to_gates(self, node, target: str, acc: List[Gate]) -> str:
        """Lower AST to 2-input gates; return the signal name producing node's value.
        The final result is wired to 'target'.
        """
        if node is None:
            return target
        kind = node[0]
        if kind == 'id':
            src = node[1]
            # Connect via a buffer (no gate); ensure final output uses src directly
            if target != src:
                # Implement as XOR with 0? Better: emit a single-wire assignment by naming convention
                # We'll just return src so caller can wire it
                return src
            return target
        if kind == 'not':
            a_sig = self._ast_to_gates(node[1], self._new_temp('n'), acc)
            acc.append(Gate(gate_type='NOT', inputs=[a_sig], output=target, instance_name=f"NOT_1_{target}"))
            return target
        # Binary ops
        op_map = {
            'and': 'AND',
            'or': 'OR',
            'xor': 'XOR'
        }
        left_sig = self._ast_to_gates(node[1], self._new_temp('l'), acc)
        right_sig = self._ast_to_gates(node[2], self._new_temp('r'), acc)
        acc.append(Gate(gate_type=op_map[kind], inputs=[left_sig, right_sig], output=target, instance_name=f"{op_map[kind]}_2_{target}"))
        return target
    
    def _parse_complex_expression_to_gates(self, expr: str, output: str) -> List[Gate]:
        """Backward-compat shim: use the generic boolean expression parser."""
        return self._parse_boolean_expr_to_gates(expr, output)
    
    def _parse_expression(self, expr: str) -> Tuple[Optional[str], List[str]]:
        """Deprecated: retained for compatibility. Use _parse_boolean_expr_to_gates instead."""
        gates = self._parse_boolean_expr_to_gates(expr, 'tmp_out')
        if not gates:
            return None, []
        g = gates[-1]
        return g.gate_type, g.inputs
    
    def _parse_complex_expression(self, expr: str) -> Tuple[Optional[str], List[str]]:
        """Deprecated shim to maintain interface; map to generic parser."""
        return self._parse_expression(expr)
    
    def _parse_module_instantiation(self, line: str) -> Optional[ModuleInstance]:
        """Parse module instantiation"""
        # Match pattern: module_name instance_name ( .port(signal), ... );
        match = re.match(r'(\w+)\s+(\w+)\s*\((.*?)\);', line, re.DOTALL)
        if match:
            module_name = match.group(1)
            instance_name = match.group(2)
            connections_str = match.group(3)
            
            # Parse connections
            connections: Dict[str, str] = {}
            for pm in re.finditer(r'\.(\w+)\s*\(\s*([^\)]+)\s*\)', connections_str):
                port = pm.group(1)
                signal = pm.group(2)
                connections[port] = signal
            
            return ModuleInstance(
                module_name=module_name,
                instance_name=instance_name,
                connections=connections
            )
        
        return None

def sanitize_signal_name(name: str) -> str:
    # Convert bus refs like a[0] -> a_0
    name = name.strip()
    name = name.replace('[', '_').replace(']', '')
    name = name.replace(':', '_')
    return name

def flatten_module_gates(module: Module, modules: Dict[str, Module]) -> List[Gate]:
    flat: List[Gate] = []
    # If this module has no instances, copy local gates; otherwise rely on flattened instances
    if not module.instances:
        for g in module.gates:
            new_g = Gate(gate_type=g.gate_type,
                         inputs=[sanitize_signal_name(s) for s in g.inputs],
                         output=sanitize_signal_name(g.output),
                         instance_name=g.instance_name)
            flat.append(new_g)
    # Inline submodules
    port_names_in = set(s.name for s in module.inputs)
    port_names_out = set(s.name for s in module.outputs)
    for inst in module.instances:
        # Virtual primitive: UNIT_DFFE maps directly to a DFF gate
        if inst.module_name == 'UNIT_DFFE':
            # Accept common port names (D/d, CLK/clk, Q/q)
            d_port = 'D' if 'D' in inst.connections else 'd'
            clk_port = 'CLK' if 'CLK' in inst.connections else 'clk'
            q_port = 'Q' if 'Q' in inst.connections else 'q'
            if d_port in inst.connections and clk_port in inst.connections and q_port in inst.connections:
                d_sig = sanitize_signal_name(inst.connections[d_port])
                clk_sig = sanitize_signal_name(inst.connections[clk_port])
                q_sig = sanitize_signal_name(inst.connections[q_port])
                flat.append(Gate(gate_type='DFF', inputs=[d_sig, clk_sig], output=q_sig, instance_name=f"{inst.instance_name}_DFF"))
            continue
        if inst.module_name not in modules:
            continue
        sub = modules[inst.module_name]
        sub_gates = flatten_module_gates(sub, modules)
        # Build port mapping from submodule port name -> connected net (sanitized)
        port_map: Dict[str, str] = {}
        for k, v in inst.connections.items():
            # k is port name, v is signal (maybe bus)
            port_map[k] = sanitize_signal_name(v)
        # Determine submodule's ports sets
        sub_ports_in = set(s.name for s in sub.inputs)
        sub_ports_out = set(s.name for s in sub.outputs)
        for g in sub_gates:
            mapped_inputs: List[str] = []
            for s in g.inputs:
                base = s
                # Map through ports if present
                if base in port_map:
                    mapped_inputs.append(port_map[base])
                else:
                    # Instance-local internal -> prefix with instance name
                    mapped_inputs.append(f"{inst.instance_name}_{base}")
            out_base = g.output
            if out_base in port_map:
                out_sig = port_map[out_base]
            else:
                out_sig = f"{inst.instance_name}_{out_base}"
            flat.append(Gate(gate_type=g.gate_type,
                             inputs=mapped_inputs,
                             output=out_sig,
                             instance_name=f"{inst.instance_name}_{g.instance_name}"))
    return flat

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
            },
            'DFF': {
                'part_number': '74HC74',
                'package': 'DIP-14',
                'gates_per_ic': 2,
                'pinout': {
                    # Dual D-type flip-flop; using only CLK and D inputs, Q output
                    # Unit A: D=2, CLK=3, Q=5; Unit B: D=12, CLK=11, Q=9
                    'gate1': {'inputs': ['2', '3'], 'output': '5'},
                    'gate2': {'inputs': ['12', '11'], 'output': '9'},
                    'vcc': '14',
                    'gnd': '7'
                }
            }
        }
    
    def map_gates_to_ics(self, gates: List[Gate]) -> List[ICInstance]:
        """Map gates to IC components with proper pin assignments"""
        ic_instances = []
        gate_counts = defaultdict(list)

        # Group gates by type (skip ALIAS - no IC needed)
        passthrough_links = []  # list of (dst, src)
        for gate in gates:
            if gate.gate_type == 'ALIAS':
                # capture for later net tie in exporter via pin_assignments-like structure
                passthrough_links.append((gate.output, gate.inputs[0]))
                continue
            gate_counts[gate.gate_type].append(gate)

        # Create IC instances
        for gate_type, gate_list in gate_counts.items():
            if gate_type in self.ic_mappings:
                mapping = self.ic_mappings[gate_type]
                gates_per_ic = mapping['gates_per_ic']
                for ic_num in range((len(gate_list) + gates_per_ic - 1) // gates_per_ic):
                    ic = ICInstance(
                        part_number=mapping['part_number'],
                        package=mapping['package'],
                        instance_id=f"U{len(ic_instances) + 1}"
                    )
                    start_idx = ic_num * gates_per_ic
                    end_idx = min(start_idx + gates_per_ic, len(gate_list))
                    ic_gates = gate_list[start_idx:end_idx]
                    gate_names = list(mapping['pinout'].keys())
                    for i, gate in enumerate(ic_gates):
                        if i < len(gate_names):
                            gate_name = gate_names[i]
                            pinout = mapping['pinout'][gate_name]
                            for j, input_signal in enumerate(gate.inputs):
                                if j < len(pinout['inputs']):
                                    pin = pinout['inputs'][j]
                                    ic.pin_assignments[pin] = input_signal
                            ic.pin_assignments[pinout['output']] = gate.output
                            gate.ic_instance = ic
                            ic.gates.append(gate)
                    ic.pin_assignments[mapping['pinout']['vcc']] = 'VCC'
                    ic.pin_assignments[mapping['pinout']['gnd']] = 'GND'
                    ic_instances.append(ic)

        # Attach passthrough_links to a synthetic ICInstance for exporter to pick up
        if passthrough_links:
            alias_ic = ICInstance(instance_id="ALIAS", part_number="ALIAS", package="N/A")
            for dst, src in passthrough_links:
                # store as pin_assignments key=dst, value=src for later net tying
                alias_ic.pin_assignments[dst] = src
            ic_instances.append(alias_ic)

        return ic_instances

class KiCadNetlistExporter:
    """KiCad netlist exporter for PCB import"""
    
    def _merge_signals_by_name(self, signals: List[Signal]) -> List[Signal]:
        """Deduplicate ports by name, keeping the widest declaration.
        Prevents emitting both aggregate (e.g., JIN_a) and bit-sliced (JIN_a_0..n) connectors.
        """
        merged: Dict[str, Signal] = {}
        for sig in signals:
            # Prefer the signal with the larger width for a given base name
            existing = merged.get(sig.name)
            if existing is None or (sig.width or 1) > (existing.width or 1):
                merged[sig.name] = sig
        return list(merged.values())
    
    def export_netlist(self, module: Module, ic_instances: List[ICInstance], output_file: str):
        """Export KiCad netlist format (.net file)"""
        # Ensure destination directory exists
        Path(output_file).parent.mkdir(parents=True, exist_ok=True)
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
            # I/O connectors (one-pin) with de-duplication
            seen_refs = set()
            inputs = self._merge_signals_by_name(module.inputs)
            outputs = self._merge_signals_by_name(module.outputs)
            for sig in inputs:
                if sig.width and sig.width > 1:
                    for i in range(sig.width):
                        ref = f"JIN_{sig.name}_{i}"
                        if ref in seen_refs: continue
                        self._write_connector_component(f, ref=ref, value="Conn_01x01", package="DIP-1")
                        seen_refs.add(ref)
                else:
                    # Always add scalar connectors (e.g., cin)
                    ref = f"JIN_{sig.name}"
                    if ref not in seen_refs:
                        self._write_connector_component(f, ref=ref, value="Conn_01x01", package="DIP-1")
                        seen_refs.add(ref)
            for sig in outputs:
                if sig.width and sig.width > 1:
                    for i in range(sig.width):
                        ref = f"JOUT_{sig.name}_{i}"
                        if ref in seen_refs: continue
                        self._write_connector_component(f, ref=ref, value="Conn_01x01", package="DIP-1")
                        seen_refs.add(ref)
                else:
                    # Always add scalar connectors (e.g., cout)
                    ref = f"JOUT_{sig.name}"
                    if ref not in seen_refs:
                        self._write_connector_component(f, ref=ref, value="Conn_01x01", package="DIP-1")
                        seen_refs.add(ref)
            for ic in ic_instances:
                if ic.part_number == 'ALIAS':
                    continue  # synthetic net-tie, not a real component
                self._write_component_netlist(f, ic)

            # Add one 0.1uF decoupling capacitor per real 74xx IC
            cap_refs: List[str] = []
            real_ics = [ic for ic in ic_instances if str(ic.part_number).startswith('74')]
            for idx, _ic in enumerate(real_ics, start=1):
                cref = f"C{idx}"
                cap_refs.append(cref)
                self._write_capacitor_component(f, ref=cref, value="0.1uF", package="C_Disc_D5.0mm_W2.5mm_P5.00mm")
            f.write("  )\n")
            
            # Write nets
            f.write("  (nets\n")
            self._write_nets(f, module, ic_instances, decoupling_caps=cap_refs)
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

    def _write_connector_component(self, f, ref: str, value: str, package: str):
        """Write a 1-pin connector as a component for I/O"""
        f.write(f"    (comp (ref {ref})\n")
        f.write(f"      (value {value})\n")
        # Use a default KiCad footprint that exists in standard libs
        f.write(f"      (footprint Connector_PinHeader_2.54mm:PinHeader_1x01_P2.54mm_Vertical)\n")
        f.write(f"      (datasheet \"\")\n")
        f.write(f"      (fields\n")
        f.write(f"        (field (name F0) \"{ref}\")\n")
        f.write(f"        (field (name F1) \"{value}\")\n")
        f.write(f"        (field (name F2) \"{package}\")\n")
        f.write(f"      )\n")
        f.write(f"      (libsource (lib \"Connector_Generic\") (part \"{value}\"))\n")
        f.write(f"      (sheetpath (names \"/\") (tstamps \"/\"))\n")
        f.write(f"      (tstamp {self._generate_timestamp()})\n")
        f.write(f"    )\n")

    def _write_capacitor_component(self, f, ref: str, value: str, package: str):
        """Write a decoupling capacitor component (2-pin)"""
        f.write(f"    (comp (ref {ref})\n")
        f.write(f"      (value {value})\n")
        # Choose a common through-hole footprint for easy prototyping
        f.write(f"      (footprint Capacitor_THT:{package})\n")
        f.write(f"      (datasheet \"\")\n")
        f.write(f"      (fields\n")
        f.write(f"        (field (name F0) \"{ref}\")\n")
        f.write(f"        (field (name F1) \"C\")\n")
        f.write(f"        (field (name F2) \"{package}\")\n")
        f.write(f"      )\n")
        f.write(f"      (libsource (lib \"Device\") (part \"C\"))\n")
        f.write(f"      (sheetpath (names \"/\") (tstamps \"/\"))\n")
        f.write(f"      (tstamp {self._generate_timestamp()})\n")
        f.write(f"    )\n")
    
    def _write_nets(self, f, module: Module, ic_instances: List[ICInstance], decoupling_caps: Optional[List[str]] = None):
        """Write net definitions"""
        # Collect all nets and their connections
        nets = defaultdict(set)
        # Collect alias links if present
        alias_links: List[Tuple[str, str]] = []
        
        # Add input/output nets to connector pins (pin 1)
        for signal in self._merge_signals_by_name(module.inputs):
            if signal.width and signal.width > 1:
                for i in range(signal.width):
                    nets[f"{signal.name}_{i}"].add((f'JIN_{signal.name}_{i}', '1'))
            else:
                nets[signal.name].add((f'JIN_{signal.name}', '1'))
        for signal in self._merge_signals_by_name(module.outputs):
            if signal.width and signal.width > 1:
                for i in range(signal.width):
                    nets[f"{signal.name}_{i}"].add((f'JOUT_{signal.name}_{i}', '1'))
            else:
                nets[signal.name].add((f'JOUT_{signal.name}', '1'))
        
        # Add internal nets from IC pin assignments
        for ic in ic_instances:
            if ic.part_number == 'ALIAS':
                for dst, src in ic.pin_assignments.items():
                    alias_links.append((dst, src))
                continue
            for pin, signal in ic.pin_assignments.items():
                if signal not in ['VCC', 'GND']:
                    nets[signal].add((ic.instance_id, pin))
        
        # Add power nets
        vcc_connections = set()
        gnd_connections = set()
        for ic in ic_instances:
            for pin, signal in ic.pin_assignments.items():
                if signal == 'VCC':
                    vcc_connections.add((ic.instance_id, pin))
                elif signal == 'GND':
                    gnd_connections.add((ic.instance_id, pin))
        
        # Write power nets
        # Add decoupling caps to power nets
        decoupling_caps = decoupling_caps or []
        for cref in decoupling_caps:
            # Capacitor pins: 1->VCC, 2->GND
            vcc_connections.add((cref, '1'))
            gnd_connections.add((cref, '2'))

        if vcc_connections:
            f.write(f"    (net (code {self._generate_net_code()}) (name \"VCC\")\n")
            for ref, pin in sorted(vcc_connections):
                f.write(f"      (node (ref {ref}) (pin {pin}))\n")
            f.write(f"    )\n")
        
        if gnd_connections:
            f.write(f"    (net (code {self._generate_net_code()}) (name \"GND\")\n")
            for ref, pin in sorted(gnd_connections):
                f.write(f"      (node (ref {ref}) (pin {pin}))\n")
            f.write(f"    )\n")
        
        # Apply alias links by merging source net into destination net
        for dst, src in alias_links:
            if src in nets:
                nets[dst] |= nets[src]
                # Also include connectors already added for src (handled above)
                # Remove the source net to avoid duplicating
                if dst != src and src in nets:
                    del nets[src]

        # Write signal nets (include even single-connection nets to expose I/O in netlist)
        for net_name, connections in nets.items():
            f.write(f"    (net (code {self._generate_net_code()}) (name \"{net_name}\")\n")
            for ref, pin in sorted(connections):
                if ref == 'ALIAS':
                    continue
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
            '74HC04': list(range(1, 15)),
            '74HC74': list(range(1, 15))
        }
        
        # Define power pins that should not be connected to GND
        power_pins = {
            '74HC08': [7, 14],  # GND=7, VCC=14
            '74HC32': [7, 14],
            '74HC86': [7, 14],
            '74HC04': [7, 14],
            '74HC74': [7, 14]
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
            if ic.part_number == 'ALIAS':
                # Place alias endpoints near each other
                for dst, src in ic.pin_assignments.items():
                    if dst not in signal_positions:
                        signal_positions[dst] = (2750, 1000)
                    if src not in signal_positions:
                        signal_positions[src] = (2800, 1000)
                continue
            for pin, signal in ic.pin_assignments.items():
                if signal not in signal_positions and signal not in ['VCC', 'GND']:
                    # Calculate pin position based on IC position and pin number
                    x, y = ic.position
                    pin_x, pin_y = self._get_pin_position(ic, pin, x, y)
                    signal_positions[signal] = (pin_x, pin_y)
        
        # Write wire connections
        for ic in ic_instances:
            if ic.part_number == 'ALIAS':
                # Draw short tie lines between alias pairs
                for dst, src in ic.pin_assignments.items():
                    if dst in signal_positions and src in signal_positions:
                        x1, y1 = signal_positions[dst]
                        x2, y2 = signal_positions[src]
                        f.write("Wire Wire Line\n")
                        f.write(f"    {x1} {y1} {x2} {y2}\n")
                continue
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
    
    # Determine top-level module: a module that is not instantiated by any other
    instantiated: Set[str] = set()
    for m in modules.values():
        for inst in m.instances:
            instantiated.add(inst.module_name)
    top_modules = [m for m in modules.values() if m.name not in instantiated]
    if not top_modules:
        # Fallback: use the first module
        top_modules = [next(iter(modules.values()))]
    
    # Ensure generated output folder exists
    gen_dir = Path(__file__).parent / 'generated'
    gen_dir.mkdir(parents=True, exist_ok=True)
    
    # Process only the top-level module
    for module in top_modules:
        name = module.name
        if args.verbose:
            print(f"\nProcessing module: {name}")
        
        # Flatten hierarchy and map gates to ICs
        flat_gates = flatten_module_gates(module, modules)
        mapper = GateToICMapper()
        ic_instances = mapper.map_gates_to_ics(flat_gates)
        
        if args.verbose:
            print(f"Generated {len(ic_instances)} IC instances")
            for ic in ic_instances:
                print(f"  - {ic.instance_id}: {ic.part_number} ({ic.package})")
                print(f"    Gates: {len(ic.gates)}")
                print(f"    Pin assignments: {ic.pin_assignments}")
        
        # Export to KiCad schematic
        exporter = KiCadExporter()
        output_file = gen_dir / f"{name}_{args.output}"
        exporter.export_schematic(module, ic_instances, str(output_file))
        
        if args.verbose:
            print(f"Exported schematic to: {output_file}")
        
        # Export KiCad netlist for PCB import
        netlist_exporter = KiCadNetlistExporter()
        netlist_file = gen_dir / f"{name}.net"
        netlist_exporter.export_netlist(module, ic_instances, str(netlist_file))
        
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
            json_file = gen_dir / f"{name}_netlist.json"
            with open(json_file, 'w') as f:
                json.dump(netlist, f, indent=2)
            
            if args.verbose:
                print(f"Exported JSON netlist to: {json_file}")

if __name__ == '__main__':
    main()
