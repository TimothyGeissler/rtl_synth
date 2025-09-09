# Verilog to PCB Converter

A Python toolchain that converts Verilog RTL designs to gate-level PCB schematics using discrete logic gate ICs. This tool bridges the gap between digital design and physical implementation by automatically generating KiCad-compatible schematics that can be fabricated as PCBs.

## Features

- **Verilog RTL Parsing**: Parses Verilog modules, ports, and gate-level operations
- **Gate-to-IC Mapping**: Maps logic gates to common 74xx series IC components
- **KiCad Export**: Generates KiCad-compatible schematic files
- **KiCad Netlist**: Exports KiCad `.net` files for direct PCB import
- **JSON Netlist**: Exports detailed netlist information in JSON format
- **Multi-Module Support**: Handles hierarchical Verilog designs
- **Component Library**: Supports AND, OR, XOR, and NOT gates with proper pin assignments

## Supported IC Components

| Gate Type | IC Part Number | Package | Gates per IC |
|-----------|----------------|---------|--------------|
| AND       | 74HC08         | DIP-14  | 4            |
| OR        | 74HC32         | DIP-14  | 4            |
| XOR       | 74HC86         | DIP-14  | 4            |
| NOT       | 74HC04         | DIP-14  | 6            |

## Installation

No special dependencies required - uses only Python standard library.

```bash
# Clone or download the verilog_to_pcb_final.py script
python3 verilog_to_pcb_final.py --help
```

## Usage

### Basic Usage

```bash
python3 verilog_to_pcb_final.py input_file.v
```

### Advanced Options

```bash
python3 verilog_to_pcb_final.py input_file.v \
    --output my_design.sch \
    --json \
    --verbose
```

### Command Line Options

- `input_file`: Input Verilog file (required)
- `-o, --output`: Output KiCad schematic file (default: output.sch)
- `--json`: Export netlist as JSON file
- `-v, --verbose`: Enable verbose output

## Example: 4-bit Adder

The tool has been tested with a 4-bit ripple carry adder design. Here's how to use it:

```bash
python3 verilog_to_pcb_final.py rtl/adder4b_sample.v -v --json
```

This will generate:
- `adder_4bit_output.sch` - KiCad schematic file
- `full_adder_output.sch` - KiCad schematic file for full adder module
- `adder_4bit.net` - KiCad netlist file for PCB import
- `full_adder.net` - KiCad netlist file for full adder module
- `adder_4bit_netlist.json` - JSON netlist with component details
- `full_adder_netlist.json` - JSON netlist for full adder module

### Input Verilog (adder4b_sample.v)

```verilog
// 4-bit Ripple Carry Adder
module adder_4bit (
    input [3:0] a,      // First 4-bit input
    input [3:0] b,      // Second 4-bit input
    input cin,          // Carry input
    output [3:0] sum,   // 4-bit sum output
    output cout         // Carry output
);
    // Internal carry signals
    wire c1, c2, c3;
    
    // Instantiate 4 full adders
    full_adder fa0 (.a(a[0]), .b(b[0]), .cin(cin), .sum(sum[0]), .cout(c1));
    full_adder fa1 (.a(a[1]), .b(b[1]), .cin(c1),  .sum(sum[1]), .cout(c2));
    full_adder fa2 (.a(a[2]), .b(b[2]), .cin(c2),  .sum(sum[2]), .cout(c3));
    full_adder fa3 (.a(a[3]), .b(b[3]), .cin(c3),  .sum(sum[3]), .cout(cout));
endmodule

// Full Adder module
module full_adder (
    input a,        // First input bit
    input b,        // Second input bit
    input cin,      // Carry input
    output sum,     // Sum output
    output cout     // Carry output
);
    // Sum = a XOR b XOR cin
    assign sum = a ^ b ^ cin;
    
    // Carry out = (a AND b) OR (cin AND (a XOR b))
    assign cout = (a & b) | (cin & (a ^ b));
endmodule
```

### Generated Output

The tool will generate IC instances for each gate operation:

**For the full_adder module:**
- **U1 (74HC86)**: XOR gate for sum calculation (a ^ b ^ cin)
- **U2 (74HC32)**: OR gate for carry calculation ((a & b) | (cin & (a ^ b)))

**JSON Netlist Example:**
```json
{
  "module_name": "full_adder",
  "inputs": [
    {"name": "a", "width": 1},
    {"name": "b", "width": 1},
    {"name": "cin", "width": 1}
  ],
  "outputs": [
    {"name": "sum", "width": 1},
    {"name": "cout", "width": 1}
  ],
  "ic_instances": [
    {
      "instance_id": "U1",
      "part_number": "74HC86",
      "package": "DIP-14",
      "pin_assignments": {
        "1": "a",
        "2": "b",
        "3": "sum",
        "14": "VCC",
        "7": "GND"
      }
    }
  ]
}
```

## How It Works

### 1. Verilog Parsing
- Extracts module definitions, ports, and internal signals
- Parses `assign` statements to identify gate operations
- Handles module instantiation for hierarchical designs

### 2. Gate Mapping
- Converts Verilog operations to discrete logic gates
- Maps gates to appropriate 74xx series IC components
- Optimizes IC usage by packing multiple gates per chip

### 3. Pin Assignment
- Assigns input/output signals to specific IC pins
- Handles power (VCC) and ground (GND) connections
- Ensures proper pin mapping based on IC datasheets

### 4. Schematic Generation
- Creates KiCad-compatible schematic files
- Places components in an organized grid layout
- Generates wire connections between components

## KiCad Integration

### Method 1: Using KiCad Netlist Files (Recommended)

The tool now generates KiCad `.net` files that can be directly imported into KiCad PCB Editor:

1. **Open KiCad PCB Editor**
2. **File → Import → Netlist**
3. **Select the generated `.net` file** (e.g., `full_adder.net`)
4. **Components will be imported** with proper footprints and connections
5. **Place components and route traces** for PCB fabrication

### Method 2: Using Schematic Files

1. Open KiCad and create a new project
2. Open the Schematic Layout Editor
3. File → Import → Import Non-KiCad Schematic
4. Select the generated `.sch` file

### Component Libraries

You may need to add the 74xx series component libraries in KiCad:
1. Preferences → Manage Symbol Libraries
2. Add the standard 74xx library
3. Ensure footprints are mapped for PCB layout

### PCB Layout

After importing the netlist:
1. **Components are automatically placed** in the PCB editor
2. **Nets are already defined** with proper connections
3. **Arrange components** for optimal routing
4. **Route the traces** for physical PCB fabrication
5. **Generate Gerber files** for PCB manufacturing

## Extending the Tool

### Adding New Gate Types

To add support for new gate types, modify the `ic_mappings` dictionary in the `GateToICMapper` class:

```python
self.ic_mappings['NAND'] = {
    'part_number': '74HC00',
    'package': 'DIP-14',
    'gates_per_ic': 4,
    'pinout': {
        'gate1': {'inputs': ['1', '2'], 'output': '3'},
        # ... additional gates
        'vcc': '14',
        'gnd': '7'
    }
}
```

### Expression Parsing

The tool currently handles:
- Simple operations: `a & b`, `a | b`, `a ^ b`, `~a`
- Complex expressions: `(a & b) | (c & d)`

To extend expression parsing, modify the `_parse_expression` and `_parse_complex_expression` methods in the `VerilogParser` class.

## Limitations

1. **Expression Complexity**: Complex nested expressions may not be optimally parsed
2. **Module Instantiation**: Hierarchical designs are parsed but not fully flattened
3. **Bus Handling**: Multi-bit signals are not yet fully supported
4. **Sequential Logic**: Only combinational logic is currently supported
5. **KiCad Versions**: Generated files target KiCad v4 format

## Future Enhancements

- [ ] Support for sequential logic (flip-flops, latches)
- [ ] Multi-bit bus handling with proper bit extraction
- [ ] Automatic component placement optimization
- [ ] Support for more IC families (CMOS, TTL variants)
- [ ] Integration with component databases for real-time availability
- [ ] PCB routing hints and constraints
- [ ] Cost optimization for IC selection
- [ ] Timing analysis integration

## Contributing

This tool is designed to be easily extensible. Key areas for contribution:

1. **Parser Improvements**: Better Verilog syntax support
2. **IC Library Expansion**: More gate types and IC families
3. **KiCad Integration**: Better schematic formatting
4. **Optimization**: Component placement and gate packing algorithms
5. **Testing**: More complex Verilog designs

## License

This tool is provided as-is for educational and experimental purposes. Please verify all generated designs before fabrication.

## Example Projects

### Digital Clock
A complete digital clock design using counters, decoders, and display drivers.

### ALU (Arithmetic Logic Unit)
A simple 4-bit ALU with add, subtract, AND, OR operations.

### Memory Decoder
Address decoders for memory interfacing projects.

## Bill of Materials (BOM) Generation

The JSON netlist can be used to generate a Bill of Materials for ordering components:

```python
# Example BOM generation from JSON netlist
import json
from collections import Counter

with open('design_netlist.json', 'r') as f:
    netlist = json.load(f)

components = [ic['part_number'] for ic in netlist['ic_instances']]
bom = Counter(components)

print("Bill of Materials:")
for part, qty in bom.items():
    print(f"{qty}x {part}")
```

This will help you order the exact components needed for your PCB build.
