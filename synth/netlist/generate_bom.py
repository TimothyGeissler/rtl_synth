#!/usr/bin/env python3
"""
Bill of Materials (BOM) Generator
Generates a BOM from the JSON netlist output of the Verilog to PCB converter.
"""

import json
import argparse
from collections import Counter
from pathlib import Path

def generate_bom(netlist_file: str, output_file: str = None):
    """Generate BOM from JSON netlist"""
    
    # Read the netlist
    with open(netlist_file, 'r') as f:
        netlist = json.load(f)
    
    # Extract component information
    components = []
    for ic in netlist['ic_instances']:
        components.append({
            'part_number': ic['part_number'],
            'package': ic['package'],
            'instance_id': ic['instance_id']
        })
    
    # Count components
    part_counts = Counter(comp['part_number'] for comp in components)
    package_info = {comp['part_number']: comp['package'] for comp in components}
    
    # Generate BOM
    bom_lines = []
    bom_lines.append("# Bill of Materials (BOM)")
    bom_lines.append(f"# Generated from: {netlist_file}")
    bom_lines.append(f"# Module: {netlist['module_name']}\n")

    # I/O Ports section
    inputs_raw = netlist.get('inputs', [])
    outputs_raw = netlist.get('outputs', [])

    def width_of(sig):
        try:
            return int(sig.get('width', 1)) if sig.get('width') else 1
        except Exception:
            return 1

    def collapse_ports(ports):
        by_name = {}
        for p in ports:
            name = p.get('name')
            if not name:
                continue
            w = width_of(p)
            if name not in by_name or w > by_name[name]:
                by_name[name] = w
        # stable order by name
        return [{ 'name': n, 'width': w } for n, w in sorted(by_name.items())]

    inputs = collapse_ports(inputs_raw)
    outputs = collapse_ports(outputs_raw)

    total_inputs = sum(width_of(s) for s in inputs)
    total_outputs = sum(width_of(s) for s in outputs)

    bom_lines.append("## I/O Ports")
    bom_lines.append(f"- **Inputs ({total_inputs} pins, {len(inputs)} ports)**:")
    for s in inputs:
        w = width_of(s)
        if w > 1:
            bom_lines.append(f"  - {s['name']}[{w-1}:0] ({w} pins)")
        else:
            bom_lines.append(f"  - {s['name']}")
    bom_lines.append(f"- **Outputs ({total_outputs} pins, {len(outputs)} ports)**:")
    for s in outputs:
        w = width_of(s)
        if w > 1:
            bom_lines.append(f"  - {s['name']}[{w-1}:0] ({w} pins)")
        else:
            bom_lines.append(f"  - {s['name']}")
    bom_lines.append("")
    
    bom_lines.append("## Components")
    bom_lines.append("| Qty | Part Number | Package | Description |")
    bom_lines.append("|-----|-------------|---------|-------------|")
    
    total_cost = 0.0
    
    for part_number, qty in sorted(part_counts.items()):
        package = package_info[part_number]
        description = get_component_description(part_number)
        estimated_cost = get_estimated_cost(part_number) * qty
        total_cost += estimated_cost
        
        bom_lines.append(f"| {qty} | {part_number} | {package} | {description} |")
    
    bom_lines.append(f"\n**Total Components:** {sum(part_counts.values())}")
    bom_lines.append(f"**Estimated Cost:** ${total_cost:.2f} USD")
    
    # Add power supply requirements
    bom_lines.append("\n## Power Supply Requirements")
    bom_lines.append("- **Voltage:** 5V DC")
    bom_lines.append("- **Current:** ~20mA per IC (typical)")
    bom_lines.append(f"- **Total Current:** ~{sum(part_counts.values()) * 20}mA")
    
    # Add additional components
    bom_lines.append("\n## Additional Components Needed")
    bom_lines.append("| Qty | Part | Description |")
    bom_lines.append("|-----|------|-------------|")
    bom_lines.append("| 1 | Power connector | 5V DC input |")
    bom_lines.append("| 1 | PCB | Custom PCB or breadboard |")
    bom_lines.append("| N | Headers/Connectors | For input/output connections |")
    bom_lines.append("| N | Decoupling capacitors | 0.1µF ceramic (one per IC) |")
    
    bom_text = '\n'.join(bom_lines)
    
    # Output BOM
    if output_file:
        with open(output_file, 'w') as f:
            f.write(bom_text)
        print(f"BOM written to: {output_file}")
    else:
        print(bom_text)

def get_component_description(part_number: str) -> str:
    """Get description for a component"""
    descriptions = {
        '74HC08': 'Quad 2-input AND gate',
        '74HC32': 'Quad 2-input OR gate', 
        '74HC86': 'Quad 2-input XOR gate',
        '74HC04': 'Hex inverter (NOT gate)'
    }
    return descriptions.get(part_number, 'Logic gate IC')

def get_estimated_cost(part_number: str) -> float:
    """Get estimated cost for a component (in USD)"""
    costs = {
        '74HC08': 0.50,
        '74HC32': 0.50,
        '74HC86': 0.55,
        '74HC04': 0.45
    }
    return costs.get(part_number, 0.50)

def main():
    parser = argparse.ArgumentParser(description='Generate BOM from Verilog to PCB netlist')
    parser.add_argument('netlist_file', help='Input JSON netlist file')
    parser.add_argument('-o', '--output', help='Output BOM file')
    
    args = parser.parse_args()
    
    if not Path(args.netlist_file).exists():
        print(f"Error: Netlist file '{args.netlist_file}' not found")
        return 1
    
    generate_bom(args.netlist_file, args.output)
    return 0

if __name__ == '__main__':
    exit(main())
