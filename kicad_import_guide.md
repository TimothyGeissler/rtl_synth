# KiCad Import Guide

This guide shows you how to import the generated KiCad netlist files into KiCad for PCB design and fabrication.

## Quick Start

1. **Generate your design:**
   ```bash
   python3 verilog_to_pcb_final.py your_design.v -v --json
   ```

2. **Import into KiCad:**
   - Open KiCad PCB Editor
   - File → Import → Netlist
   - Select your generated `.net` file

## Step-by-Step KiCad Import

### 1. Open KiCad PCB Editor

Launch KiCad and open the PCB Layout Editor (not the Schematic Editor).

### 2. Import Netlist

1. Go to **File → Import → Netlist**
2. Browse to your generated `.net` file (e.g., `full_adder.net`)
3. Click **Open**

### 3. Verify Components

After import, you should see:
- **Components placed** in the PCB editor
- **Nets defined** with connection information
- **Footprints assigned** (DIP-14 for most 74xx series ICs)

### 4. Component Placement

1. **Drag components** to arrange them logically
2. **Consider routing** - place components to minimize trace crossings
3. **Leave space** for power connections and input/output connectors

### 5. Routing

1. **Use the routing tool** to connect components
2. **Follow the net connections** shown in the netlist
3. **Add power traces** for VCC and GND connections
4. **Add input/output connectors** as needed

## Example: Full Adder PCB

For the full adder example:

### Components to Place:
- **U1**: 74HC86 (XOR gate) - DIP-14 package
- **U2**: 74HC32 (OR gate) - DIP-14 package

### Connections to Route:
- **Input A**: Connect to pin 1 of both U1 and U2
- **Input B**: Connect to pin 2 of U1
- **Input Cin**: Connect to pin 2 of U2 (via internal logic)
- **Output Sum**: Connect from pin 3 of U1
- **Output Cout**: Connect from pin 3 of U2
- **Power**: VCC to pin 14, GND to pin 7 of both ICs

### Additional Components Needed:
- **Power connector** (2-pin header for 5V and GND)
- **Input connectors** (3-pin header for A, B, Cin)
- **Output connectors** (2-pin header for Sum, Cout)
- **Decoupling capacitors** (0.1µF ceramic, one per IC)

## Troubleshooting

### "Component not found" errors:
- Ensure the 74xx library is installed in KiCad
- Check that footprints are properly mapped
- Verify the part numbers match KiCad's library

### Missing connections:
- Check that all nets are properly defined in the `.net` file
- Verify pin assignments match the IC datasheet
- Ensure power connections (VCC/GND) are included

### Footprint issues:
- The tool uses standard DIP footprints
- If you need different packages, modify the footprint mapping in the code
- Ensure the footprint library is available in KiCad

## Advanced Usage

### Custom Footprints

To use different footprints, modify the `_get_footprint` method in the `KiCadNetlistExporter` class:

```python
def _get_footprint(self, package: str) -> str:
    footprints = {
        'DIP-14': 'Package_DIP:DIP-14_W7.62mm',
        'DIP-16': 'Package_DIP:DIP-16_W7.62mm',
        'SOIC-14': 'Package_SO:SOIC-14_3.9x8.7mm_P1.27mm',
        # Add your custom footprints here
    }
    return footprints.get(package, 'Package_DIP:DIP-14_W7.62mm')
```

### Adding Connectors

For a complete PCB, you'll need to add connectors manually in KiCad:

1. **Add connector components** to your design
2. **Connect them to the appropriate nets**
3. **Assign footprints** (headers, terminal blocks, etc.)

### Power Distribution

Consider adding:
- **Power planes** for VCC and GND
- **Decoupling capacitors** near each IC
- **Power indicator LED** with current-limiting resistor

## Manufacturing

Once your PCB is designed:

1. **Run Design Rule Check (DRC)** in KiCad
2. **Generate Gerber files** (File → Fabrication Outputs → Gerbers)
3. **Generate drill files** (File → Fabrication Outputs → Drill Files)
4. **Send to PCB manufacturer** (JLCPCB, PCBWay, etc.)

## Example PCB Layout Tips

### For Simple Designs:
- Use **single-sided PCB** to reduce cost
- Place components in a **logical flow** (inputs → logic → outputs)
- Use **wide traces** for power connections
- Add **test points** for debugging

### For Complex Designs:
- Consider **double-sided PCB** for better routing
- Use **ground plane** for better signal integrity
- Add **bypass capacitors** for each IC
- Include **jumper options** for configuration

This guide should get you started with importing your Verilog designs into KiCad for PCB fabrication!
