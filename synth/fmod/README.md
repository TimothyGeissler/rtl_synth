# Functional Model (fmod)

A lightweight functional simulator for gate-level 74xx IC designs. It can:

- Parse KiCad `.net` netlists (exported by the converter) to build a circuit
- Drive input stimuli from simple test vector files
- Propagate signals through per-IC functional models and verify outputs

## Layout

- `component_base.h`: Shared `Component` interface and logic level enum
- `components/`: One class per IC
- `components.h`: Aggregator header including all ICs
- `fmodel.h/.cpp`: Functional model framework
- `main.cpp`: CLI entrypoint
- `test_vectors/`: Sample test vector files (full_adder, adder_4bit)

## Build

```bash
cd synth/fmod
make
```

This produces `fmodel_sim`.

## Run

Usage:

```bash
./fmodel_sim <netlist_file(.net)> <test_vectors_file>
```

Examples:

```bash
# Single-bit full adder
./fmodel_sim ../netlist/full_adder.net test_vectors/full_adder_tests.txt

# 4-bit ripple-carry adder
./fmodel_sim ../netlist/adder_4bit.net test_vectors/adder_4bit_tests.txt
```

## Netlist support

- `.net` KiCad netlists are parsed to create signals and components.
- Power rails are forced by default: `VCC=HIGH`, `GND=LOW` per test.
- I/O ports are emitted as one-pin connectors by the generator; the simulator ignores non-74xx parts.
- Explicit pin mapping is used for multi-gate devices (e.g., 74HC86), ensuring internal XOR chains propagate correctly.

## Test vectors

Text format in `test_vectors/*.txt`:

```
[Test Description]
a = 0
b = 1
cin = 0
sum = 1
cout = 0
```

Inputs are `a`, `b`, `cin`, etc.; expected outputs listed per vector are checked against the simulated net values.

## Extending

- Add a new IC: create `components/<your_ic>.h/.cpp` implementing `Component` methods, include in `components.h`, and add a factory in `initializeComponentFactories()` in `fmodel.cpp`.
- Add pin mapping rules if your device has non-uniform output pins.

## Notes / Limitations

- The simulator is combinational and eventless; it iterates a few times per vector until signals settle.
- `.net` parsing is pragmatic—sufficient for the generated files. Complex hand-authored `.net` may require adjustments.
