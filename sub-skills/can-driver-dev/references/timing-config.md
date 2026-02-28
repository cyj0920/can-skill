# CAN Bit Timing Configuration

## Overview

CAN bit timing determines the baud rate and sample point position.
Proper configuration is critical for reliable communication.

## Bit Time Structure

```
 ┌── Sync ──┬──── Prop_Seg ────┬──── Phase_Seg1 ────┬── Phase_Seg2 ──┐
 │          │                  │                    │                │
 └── 1 Tq ──┴─── Prop_Tq ──────┴──── PS1_Tq ────────┴──── PS2_Tq ───┘
                                                               ↑
                                                          Sample Point
```

Total bit time: `Tq × (1 + Prop_Seg + Phase_Seg1 + Phase_Seg2)`

## Key Parameters

| Parameter | Description |
|-----------|-------------|
| Tq | Time quantum, derived from prescaler |
| Sync_Seg | Always 1 Tq |
| Prop_Seg | Propagation delay compensation |
| Phase_Seg1 | Phase buffer 1 |
| Phase_Seg2 | Phase buffer 2 |
| SJW | Synchronization Jump Width |

## Baud Rate Calculation

```
Baud_Rate = CAN_Clock / (Prescaler × Total_Tq)

Where Total_Tq = 1 + Prop_Seg + Phase_Seg1 + Phase_Seg2
```

### Example: 500 kbps @ 36 MHz APB Clock

```
Target: 500 kbps
APB Clock: 36 MHz
Desired Total_Tq: 16 (typical)

Prescaler = 36 MHz / (500 kHz × 16) = 4.5 ≈ 4
Actual Baud = 36 MHz / (4 × 16) = 562.5 kbps (error: 12.5%)

Better approach:
Prescaler = 9
Total_Tq = 8
Actual Baud = 36 MHz / (9 × 8) = 500 kbps ✓
```

## Sample Point

Sample point position: `(1 + Prop_Seg + Phase_Seg1) / Total_Tq`

**Recommended sample point: 87.5%**

For Total_Tq = 16:
- Prop_Seg + Phase_Seg1 = 14 Tq
- Phase_Seg2 = 2 Tq
- Sample point = 15/16 = 93.75%

For Total_Tq = 8:
- Prop_Seg + Phase_Seg1 = 7 Tq
- Phase_Seg2 = 1 Tq
- Sample point = 7/8 = 87.5% ✓

## Propagation Delay

### Components of Propagation Delay

```
Total Propagation Delay = 2 × (Transceiver_Delay + Cable_Delay + Isolator_Delay)

Where:
- Transceiver_Delay: ~200-250 ns typical
- Cable_Delay: ~5 ns/m
- Isolator_Delay: varies by device
```

### Prop_Seg Requirement

```
Prop_Seg ≥ Total_Prop_Delay / Tq

Example for 40m cable @ 1 Mbps:
- Cable delay: 40m × 5 ns/m = 200 ns
- Transceiver loop: 250 ns × 2 = 500 ns
- Total: 700 ns
- Tq @ 1 Mbps with 8 Tq total: 125 ns
- Prop_Seg ≥ 700/125 = 5.6 → 6 Tq minimum
```

### Impact of Isolation

Isolation adds propagation delay to the round-trip time:

| Isolation Type | Added Delay |
|----------------|-------------|
| None | 0 ns |
| Digital isolator | 10-50 ns |
| Optocoupler | 100-500 ns |
| Isolated transceiver | 250-350 ns |

**Implication**: Isolated systems may need:
- Later sample point
- Lower baud rate
- Shorter cable length

## Clock Tolerance Calculation

### Oscillator Tolerance Requirements

CAN protocol specifies maximum oscillator tolerance:

```
df = min( SJW / (2 × 10 × NBT),  SJW / (2 × (13 + PS2) )

Where:
- df = oscillator tolerance
- SJW = Synchronization Jump Width (in Tq)
- NBT = Nominal Bit Time (Total Tq)
- PS2 = Phase Segment 2 (in Tq)
```

### Practical Guidelines

| Clock Source | Typical Tolerance | Suitable for CAN |
|--------------|-------------------|------------------|
| Crystal oscillator | 0.01% - 0.1% | Yes (preferred) |
| Ceramic resonator | 0.1% - 0.5% | Yes |
| Internal RC | 1% - 3% | No (typically) |
| External TCXO | 0.001% - 0.01% | Yes (high precision) |

**Maximum tolerance**: 1.58% (theoretical), recommend < 0.5%

### Tolerance Budget Example

```
Crystal: ±100 ppm (0.01%)
Temperature drift: ±50 ppm (0.005%)
Aging: ±10 ppm/year (0.001%)
Power supply variation: ±20 ppm (0.002%)
Total: ~180 ppm (0.018%)

Conclusion: Well within tolerance for any SJW ≥ 1
```

## Register Configuration

### STM32 bxCAN (BTR Register)

```
BTR:
  [31:30] SILM: Silent mode
  [29:30] LBKM: Loopback mode
  [25:24] SJW[1:0]: Jump width (1-4 Tq)
  [22:20] TS2[2:0]: Phase_Seg2 - 1 (1-8 Tq)
  [19:16] TS1[3:0]: Prop_Seg + Phase_Seg1 - 1 (1-16 Tq)
  [9:0]   BRP[9:0]: Prescaler - 1 (1-1024)
```

### Example Configuration

```c
// 500 kbps @ 36 MHz APB, 87.5% sample point
// Prescaler = 9, TS1 = 6, TS2 = 1, SJW = 1
CAN->BTR = (9 - 1) |           // BRP = 9
           ((6 - 1) << 16) |   // TS1 = 6 (Prop + PS1 = 7)
           ((1 - 1) << 20) |   // TS2 = 1
           ((1 - 1) << 24);    // SJW = 1
// Total Tq = 1 + 6 + 1 = 8
// Sample point = (1 + 6) / 8 = 87.5%
```

## Common Baud Rates

| Baud Rate | APB Clock | Prescaler | TS1 | TS2 | Sample |
|-----------|-----------|-----------|-----|-----|--------|
| 1 Mbps | 36 MHz | 4 | 6 | 1 | 87.5% |
| 500 kbps | 36 MHz | 9 | 6 | 1 | 87.5% |
| 250 kbps | 36 MHz | 18 | 6 | 1 | 87.5% |
| 125 kbps | 36 MHz | 36 | 6 | 1 | 87.5% |
| 500 kbps | 42 MHz | 21 | 1 | 1 | 75.0% |
| 500 kbps | 48 MHz | 6 | 12 | 2 | 87.5% |
| 500 kbps | 72 MHz | 9 | 12 | 2 | 87.5% |
| 500 kbps | 80 MHz | 10 | 12 | 2 | 87.5% |
| 500 kbps | 84 MHz | 21 | 6 | 1 | 87.5% |

## Long Bus / High Speed Configuration

### For Maximum Distance at 1 Mbps

1. Set sample point to latest possible (~93%)
2. Increase Prop_Seg for propagation delay
3. Minimize Phase_Seg2

```c
// 1 Mbps @ 36 MHz, optimized for 40m cable
// Latest sample point configuration
CAN->BTR = (4 - 1) |           // BRP = 4
           ((14 - 1) << 16) |  // TS1 = 14 (Prop + PS1)
           ((1 - 1) << 20) |   // TS2 = 1
           ((2 - 1) << 24);    // SJW = 2 (extra tolerance)
// Total Tq = 1 + 14 + 1 = 16
// Sample point = 93.75%
// Allows ~900 ns propagation delay
```

### Isolated System Configuration

```c
// 500 kbps @ 36 MHz, isolated transceiver
// Allow extra propagation delay for isolation
CAN->BTR = (9 - 1) |           // BRP = 9
           ((8 - 1) << 16) |   // TS1 = 8 (increased Prop_Seg)
           ((1 - 1) << 20) |   // TS2 = 1
           ((2 - 1) << 24);    // SJW = 2
// Total Tq = 1 + 8 + 1 = 10
// Sample point = 90%
// Extra Prop_Seg accommodates isolator delay
```

## SJW Selection Guidelines

| Condition | SJW Value |
|-----------|-----------|
| Crystal oscillator, short bus | 1 Tq |
| Crystal oscillator, long bus | 1-2 Tq |
| Ceramic resonator | 2 Tq |
| Isolated system | 2-3 Tq |
| Poor clock source | Maximum (3-4 Tq) |

**Trade-off**: Larger SJW improves tolerance but reduces ability to
compensate for phase errors.

## MCU-Specific Timing Tables

### STM32F1/F4 Series

| APB1 Clock | 1 Mbps | 500 kbps | 250 kbps |
|------------|--------|----------|----------|
| 36 MHz | BRP=4, TS1=6, TS2=1 | BRP=9, TS1=6, TS2=1 | BRP=18, TS1=6, TS2=1 |
| 42 MHz | BRP=5, TS1=5, TS2=2 | BRP=21, TS1=1, TS2=1 | BRP=42, TS1=6, TS2=1 |
| 48 MHz | BRP=3, TS1=12, TS2=3 | BRP=6, TS1=12, TS2=2 | BRP=12, TS1=12, TS2=2 |

### STM32H7 Series (FDCAN)

| Clock | 1 Mbps (Nominal) | 500 kbps (Nominal) | 2 Mbps (Data) |
|-------|------------------|--------------------|---------------|
| 80 MHz | DIV=10, TSEG1=6, TSEG2=1 | DIV=20, TSEG1=6, TSEG2=1 | DIV=5, TSEG1=2, TSEG2=1 |

### NXP S32K Series

| Clock | 500 kbps | 250 kbps |
|-------|----------|----------|
| 48 MHz | PRE=12, PSEG1=5, PSEG2=2 | PRE=24, PSEG1=5, PSEG2=2 |
| 80 MHz | PRE=20, PSEG1=5, PSEG2=2 | PRE=40, PSEG1=5, PSEG2=2 |

## Troubleshooting Timing Issues

### Symptom: Intermittent Stuff Errors

**Causes**:
- Clock tolerance exceeded
- Sample point too early/late
- Propagation delay too long

**Solutions**:
1. Verify clock accuracy
2. Adjust sample point position
3. Increase SJW
4. Reduce cable length or baud rate

### Symptom: Works at Low Speed, Fails at High Speed

**Causes**:
- Propagation delay exceeds Prop_Seg
- Signal integrity issues at high frequency
- Insufficient setup time

**Solutions**:
1. Increase Prop_Seg
2. Later sample point
3. Improve cable quality
4. Add termination

## Tools

Use `scripts/can_bit_timing.py` to calculate optimal parameters:

```bash
python scripts/can_bit_timing.py --clock 36000000 --baud 500000

# With isolation delay
python scripts/can_bit_timing.py --clock 36000000 --baud 500000 --isolated

# For long bus
python scripts/can_bit_timing.py --clock 36000000 --baud 500000 --cable-length 100
```