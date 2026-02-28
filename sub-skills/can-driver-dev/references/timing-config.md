# CAN Bit Timing Configuration

## Overview

CAN bit timing determines the baud rate and sample point position.

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

## Propagation Delay

Consider propagation delay for long cables:
- Light speed in cable ~5 ns/m
- Transceiver delay ~200 ns each
- Round trip = 2 × (cable_delay + transceiver_delay)

Prop_Seg must be ≥ round_trip / Tq

## Tools

Use `scripts/can_bit_timing.py` to calculate optimal parameters:

```bash
python scripts/can_bit_timing.py --clock 36000000 --baud 500000
```
