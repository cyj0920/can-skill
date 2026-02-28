# CAN Signal Analysis

## Overview

Analyzing CAN bus signals requires understanding of physical layer
characteristics and their impact on communication reliability.

## Signal Levels

### Differential Signaling

CAN uses differential voltage between CAN_H and CAN_L:

| State | CAN_H | CAN_L | Differential |
|-------|-------|-------|--------------|
| Recessive (1) | 2.5V | 2.5V | 0V |
| Dominant (0) | 3.5V | 1.5V | 2V |

### Measurement Points

```
        Recessive     Dominant     Recessive
           |             |             |
CAN_H  ────┤          ___┤          ___┤
        2.5V        3.5V        2.5V
           
CAN_L  ────┤         ───┤         ───┤
        2.5V        1.5V        2.5V

Diff   ────┤          ___┤          ___┤
         0V         2V           0V
```

## Key Signal Parameters

### 1. Edge Slope (Slew Rate)

```
         ┌───── Dominant
        /|
       / | Rise time (tr)
──────  ─┴───────── Recessive
         ↑
      10%-90% points
```

**Typical values:**
- Fast transceiver: 3-50 ns/V
- Slow transceiver: 50-200 ns/V

**Problems:**
- Too fast: Ringing, EMI
- Too slow: Timing errors

### 2. Overshoot and Ringing

```
         ___
        /   \   Ringing
    ___/     \___
___/             \___
```

**Acceptable:** < 10% overshoot
**Causes:** Impedance mismatch, long stubs

### 3. Symmetry

TX dominant and recessive times should be equal.

**Check:**
- Measure dominant pulse width
- Compare to nominal bit time
- Variation < 5% acceptable

## Bit Timing Analysis

### Sample Point

The sample point should be at the stable part of the bit:

```
Bit:  |←── Prop ──→|←─ PS1 ─→|← PS2 →|
      |            |          S       |
      └────────────────────────────────┘
      ↑                         ↑
    Start                    Sample
```

**Ideal position:** 87.5% of bit time

### Eye Diagram

Capture multiple bits overlayed:

```
    ┌───┐       ┌───┐
    │   │       │   │
────┘   └───────┘   └────
        ↑
    "Eye" opening
    
Good signal: Large eye opening
Bad signal: Small/closed eye
```

## Common Signal Issues

### 1. Reflection

**Symptoms:**
- Ringing on edges
- Multiple transitions
- Random bit errors

**Causes:**
- Missing termination
- Long stubs
- Impedance mismatch

**Solution:**
- Add 120Ω termination at both ends
- Minimize stub length (< 0.3m)
- Use proper cable

### 2. Common Mode Noise

**Symptoms:**
- CAN_H and CAN_L both noisy
- Differential clean but errors

**Causes:**
- Ground differences
- EMI pickup
- Poor shielding

**Solution:**
- Improve grounding
- Add common mode choke
- Use shielded cable

### 3. Asymmetric Edges

**Symptoms:**
- Rise/fall times different
- Duty cycle distortion

**Causes:**
- Transceiver issues
- Bus loading imbalance
- Damaged driver

**Solution:**
- Check transceiver
- Verify bus load
- Replace driver

### 4. Slow Edges

**Symptoms:**
- Rounded corners
- Low slew rate
- Timing errors at high baud

**Causes:**
- High bus capacitance
- Too many nodes
- Wrong transceiver

**Solution:**
- Reduce bus length
- Remove nodes
- Use faster transceiver

## Measurement Techniques

### Oscilloscope Setup

```
Single-ended measurement:
CH1 → CAN_H
CH2 → CAN_L
GND → Bus GND

Differential measurement:
CH1 → CAN_H
CH2 → CAN_L (inverted)
Math → CH1 - CH2
```

### Trigger Settings

```
Trigger source: Math (differential)
Trigger level: 1V (between recessive and dominant)
Trigger mode: Normal
Holdoff: Auto
```

### Time Base Selection

| Baud Rate | Time/Div | Sample Rate |
|-----------|----------|-------------|
| 1 Mbps | 1 μs | 100 MS/s |
| 500 kbps | 2 μs | 50 MS/s |
| 250 kbps | 5 μs | 20 MS/s |

## Signal Quality Checklist

| Parameter | Good | Marginal | Bad |
|-----------|------|----------|-----|
| Recessive diff | 0V ± 0.1V | ± 0.3V | > 0.5V |
| Dominant diff | 2V ± 0.2V | ± 0.5V | < 1V |
| Rise time | < 100 ns | 100-200 ns | > 200 ns |
| Fall time | < 100 ns | 100-200 ns | > 200 ns |
| Overshoot | < 5% | 5-10% | > 10% |
| Eye opening | > 80% | 50-80% | < 50% |
