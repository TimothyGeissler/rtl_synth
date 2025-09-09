# Bill of Materials (BOM)
# Generated from: generated/adder_4bit_netlist.json
# Module: adder_4bit

## I/O Ports
- **Inputs (9 pins, 3 ports)**:
  - a[3:0] (4 pins)
  - b[3:0] (4 pins)
  - cin
- **Outputs (5 pins, 2 ports)**:
  - cout
  - sum[3:0] (4 pins)

## Components
| Qty | Part Number | Package | Description |
|-----|-------------|---------|-------------|
| 2 | 74HC08 | DIP-14 | Quad 2-input AND gate |
| 1 | 74HC32 | DIP-14 | Quad 2-input OR gate |
| 3 | 74HC86 | DIP-14 | Quad 2-input XOR gate |

**Total Components:** 6
**Estimated Cost:** $3.15 USD

## Power Supply Requirements
- **Voltage:** 5V DC
- **Current:** ~20mA per IC (typical)
- **Total Current:** ~120mA

## Additional Components Needed
| Qty | Part | Description |
|-----|------|-------------|
| 1 | Power connector | 5V DC input |
| 1 | PCB | Custom PCB or breadboard |
| N | Headers/Connectors | For input/output connections |
| N | Decoupling capacitors | 0.1µF ceramic (one per IC) |