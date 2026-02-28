# CAN Timing Debug

## Overview

Timing issues are a common cause of CAN communication problems,
especially when nodes from different manufacturers are mixed.

## Baud Rate Verification

### Method 1: Oscilloscope

```
1. Capture CAN_H or differential signal
2. Measure bit width at dominant level
3. Calculate: Baud = 1 / bit_width

Example:
Measured bit width = 2 μs
Baud rate = 1 / 2μs = 500 kbps
```

### Method 2: Known Good Message

```
1. Capture a known message
2. Count bits from SOF to EOF
3. Measure total time
4. Calculate: Baud = bits / time

Standard frame (11-bit ID, 8 data bytes):
Bits = 1 + 12 + 6 + 64 + 16 + 2 + 7 = 108 bits (approx)
```

## Sample Point Verification

### Check Sample Position

```
1. Identify start of bit (edge)
2. Find sample point timing
3. Calculate: Position = sample_time / bit_time

Target: 87.5%
```

### Mismatch Symptoms

If nodes have different sample points:
- Some nodes receive correctly, others don't
- Errors at specific bit patterns
- Communication works one direction only

## Timing Parameters Check

### Prescaler Verification

```c
// Calculate expected baud
uint32_t apb_clock = 36000000;  // Get from system config
uint32_t prescaler = (CAN->BTR & 0x3FF) + 1;
uint32_t total_tq = 1 + ((CAN->BTR >> 16) & 0xF) + 1 + 
                    ((CAN->BTR >> 20) & 0x7) + 1;
uint32_t baud = apb_clock / (prescaler * total_tq);
```

### Common Baud Rate Issues

| Issue | Symptom | Solution |
|-------|---------|----------|
| APB clock wrong | Wrong baud | Verify system clock config |
| Prescaler error | Baud off | Recalculate prescaler |
| Tq misconfiguration | Sample point error | Adjust TS1/TS2 |

## Clock Source Issues

### PLL Configuration

Verify PLL settings match expected APB clock:

```c
// Example for STM32
// System clock = 72 MHz
// APB1 clock = 36 MHz (max for CAN)

RCC->CFGR &= ~RCC_CFGR_PPRE1;    // Clear
RCC->CFGR |= RCC_CFGR_PPRE1_2;   // ÷2 → 36 MHz
```

### External Crystal

| Problem | Symptom |
|---------|---------|
| Wrong crystal | All timing off |
| Crystal drift | Baud changes with temperature |
| Missing crystal | Default to internal RC |

## Propagation Delay Issues

### Long Cable Problems

```
Propagation delay = cable_length × 5 ns/m

Total round trip = 2 × (cable_delay + transceiver_delay)

Prop_Seg must be >= round_trip / Tq
```

### Example Calculation

```
Cable length: 100m
Transceiver delay: 200 ns each

Cable delay = 100m × 5 ns/m = 500 ns
Round trip = 2 × (500 + 200) = 1400 ns

For 500 kbps (Tq = 250 ns):
Prop_Seg >= 1400 ns / 250 ns = 5.6 → 6 Tq
```

## Multi-node Timing

### All Nodes Must Match

| Parameter | Must Match |
|-----------|------------|
| Baud rate | Exactly |
| Sample point | Within tolerance |
| SJW | Can differ |

### Tolerance Calculation

```
Clock tolerance = MIN(SJW / (2 × Total_Tq), 
                      SJW / (13 × Total_Tq - 1))

For Tq = 16, SJW = 1:
Tolerance ≈ 0.3% to 0.5%
```

## Debug Procedure

### Step 1: Verify Individual Nodes

```
For each node:
1. Check APB clock frequency
2. Verify prescaler setting
3. Calculate expected baud
4. Compare with specification
```

### Step 2: Compare Nodes

```
1. Calculate baud for each node
2. Verify all match within tolerance
3. Check sample point positions
4. Verify SJW settings
```

### Step 3: Real-time Measurement

```
1. Connect oscilloscope
2. Capture communication
3. Measure actual bit width
4. Compare to expected
```

## Quick Reference: Timing Formulas

```
Baud Rate = CAN_Clock / (Prescaler × Total_Tq)

Total_Tq = 1 + Prop_Seg + Phase_Seg1 + Phase_Seg2

Sample Point = (1 + Prop_Seg + Phase_Seg1) / Total_Tq

SJW Range = 1 to MIN(4, Phase_Seg1, Phase_Seg2)

Clock Tolerance = MIN(SJW / (2 × Total_Tq), 
                      SJW / (13 × Total_Tq - 1))
```

## Troubleshooting Table

| Symptom | Check | Fix |
|---------|-------|-----|
| No communication | Baud rate | Verify all nodes same |
| Random errors | Sample point | Align all nodes |
| Long cable fails | Prop_Seg | Increase Prop_Seg |
| Temperature issues | Crystal | Use TCXO |
| Mixed vendors | Timing params | Standardize settings |
