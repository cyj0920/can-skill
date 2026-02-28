# CAN Protocol Basics

## Overview

CAN (Controller Area Network) is a serial communication protocol designed for
robust communication in automotive and industrial environments.

## History

CAN was developed by Bosch in the late 1980s for automotive applications.
Standardized as ISO 11898 (high-speed) and ISO 11519 (low-speed).

## CAN Bus Structures

### Closed-Loop (ISO 11898) - High Speed

```
     120Ω                              120Ω
  ┌──/\/\/\──┐                    ┌──/\/\/\──┐
  │          │                    │          │
Node A     CAN_H ═══════════════ CAN_H     Node B
           CAN_L ═══════════════ CAN_L
  │          │                    │          │
  └──────────┘                    └──────────┘
```

- **Speed**: 125 kbps to 1 Mbps
- **Max length**: 40m @ 1 Mbps
- **Termination**: 120Ω at each end (total ~60Ω)
- **Use case**: Automotive, industrial high-speed

### Open-Loop (ISO 11519-2) - Low Speed

```
        2.2kΩ                              2.2kΩ
     ┌──/\/\/\──┐                     ┌──/\/\/\──┐
     │          │                     │          │
Node A        CAN_H ═══════════════ CAN_H      Node B
              CAN_L ═══════════════ CAN_L
```

- **Speed**: Up to 125 kbps
- **Max length**: 1000m @ 40 kbps
- **Termination**: 2.2kΩ series resistor at each node
- **Use case**: Body electronics, low-speed networks

### Physical Layer Comparison

| Parameter | ISO 11898 (High Speed) | ISO 11519-2 (Low Speed) |
|-----------|------------------------|-------------------------|
| Speed | 125k-1M bps | ≤125k bps |
| Max length | 40m @ 1Mbps | 1000m @ 40kbps |
| Termination | 120Ω parallel | 2.2kΩ series |
| Recessive V_diff | ~0V | <0V |
| Dominant V_diff | ~2V | >2V |

## Key Features

- Multi-master architecture
- Message-based communication
- Built-in error detection
- Automatic arbitration
- High noise immunity
- No clock signal (asynchronous)
- Wired-AND logic (domant overrides recessive)

## Frame Types

| Frame Type | Purpose |
|------------|---------|
| Data Frame | Transmit data |
| Remote Frame | Request data |
| Error Frame | Signal error |
| Overload Frame | Delay next frame |
| Inter-frame Space | Separate frames |

## Data Frame Structure

### Standard Format (11-bit ID)

```
┌─────┬────────────┬─────────┬──────────┬─────┬─────┬─────┬─────┐
│ SOF │ Identifier │ Control │   Data   │ CRC │ ACK │ EOF │ IFS │
│ 1b  │   11 bits  │  6 bits │ 0-8 bytes│ 15b │  2b │  7b │  3b │
└─────┴────────────┴─────────┴──────────┴─────┴─────┴─────┴─────┘

Total (no stuffing): ~111 bits for 8-byte data
With bit stuffing: ~130 bits average
```

### Extended Format (29-bit ID)

```
┌─────┬──────────────┬──────────────┬─────────┬──────────┬─────┬─────┬─────┬─────┐
│ SOF │ Base ID (11b)│ SRR IDE ExtID│ Control │   Data   │ CRC │ ACK │ EOF │ IFS │
└─────┴──────────────┴──────────────┴─────────┴──────────┴─────┴─────┴─────┴─────┘

Total (no stuffing): ~135 bits for 8-byte data
With bit stuffing: ~160 bits average
```

### Frame Fields Explained

| Field | Bits | Description |
|-------|------|-------------|
| SOF | 1 | Start of Frame (dominant) |
| Identifier | 11/29 | Message ID, determines priority |
| RTR | 1 | Remote Transmission Request |
| IDE | 1 | Identifier Extension |
| DLC | 4 | Data Length Code (0-8 bytes) |
| Data | 0-64 | Payload |
| CRC | 15 | Cyclic Redundancy Check |
| ACK | 2 | Acknowledge slot + delimiter |
| EOF | 7 | End of Frame (7 recessive) |
| IFS | 3 | Inter-frame Space |

## Bit Encoding

- **Dominant**: Logical 0, drives bus to ~0V differential
- **Recessive**: Logical 1, bus at ~2.5V differential (idle state)
- Dominant overrides recessive (wired-AND)

### Voltage Levels (ISO 11898)

| State | CAN_H | CAN_L | Differential |
|-------|-------|-------|--------------|
| Recessive (idle) | 2.5V | 2.5V | 0V |
| Dominant (active) | 3.5V | 1.5V | 2V |

## Arbitration

- Non-destructive bitwise arbitration
- Lower ID = Higher priority
- ID 0x000 has highest priority
- Winner continues transmission, losers retry

### Arbitration Process

```
Time →  SOF  ID10 ID9 ID8 ID7 ID6 ID5 ID4 ID3 ...
Node A:  0    0    0   1   0   1   0   0   1  ... (wins)
Node B:  0    0    0   1   0   1   0   1   x  ... (loses at ID3)
                          ↑
                    Node B sends recessive, sees dominant
                    Node B switches to receive mode
```

### Priority Rules

1. Lower ID wins arbitration
2. Data frame has higher priority than remote frame (same ID)
3. Standard format has higher priority than extended format (same base ID)

## Error Detection

| Mechanism | Coverage |
|-----------|----------|
| Bit Monitor | TX vs RX comparison |
| Bit Stuffing | 5 same bits = error |
| CRC Check | 15-bit polynomial |
| ACK Check | No ACK = error |
| Form Check | Fixed format fields |

## Bit Stuffing Mechanism

CAN uses bit stuffing to ensure enough edges for synchronization:

**Rule**: After 5 consecutive bits of the same polarity, the transmitter
inserts a stuff bit of opposite polarity.

```
Data bits:    0 0 0 0 0 | 1 | 1 1 1 1 1 | 0
                  ↑         ↑
              stuff bit  stuff bit
```

**Scope**: Applies from SOF to CRC delimiter (exclusive)

**Impact**:
- Adds ~20% overhead on average
- Enables DC-free transmission
- Essential for clock synchronization

## Bit Timing

### Bit Time Structure

A CAN bit is divided into 4 segments:

```
 ┌── SYNC ──┬──── PROP ────┬──── PS1 ────┬──── PS2 ────┐
 │    1 Tq  │    Prop_Tq   │   PS1_Tq    │   PS2_Tq    │
 └──────────┴──────────────┴─────────────┴────────────┘
                                             ↑
                                        Sample Point
```

| Segment | Tq Range | Purpose |
|---------|----------|---------|
| SYNC_SEG | 1 (fixed) | Synchronization |
| PROP_SEG | 1-8 | Propagation delay compensation |
| PHASE_SEG1 | 1-8 | Phase buffer, can be extended |
| PHASE_SEG2 | 1-8 | Phase buffer, can be shortened |

### Time Quantum (Tq)

```
Tq = Prescaler / CAN_Clock
Bit_Time = Tq × (1 + PROP + PS1 + PS2)
Baud_Rate = 1 / Bit_Time
```

### Sample Point

```
Sample_Point = (1 + PROP + PS1) / Total_Tq
Recommended: 87.5%
```

### Synchronization

**Hardware Synchronization**:
- Occurs at SOF detection (bus idle → dominant edge)
- SYNC_SEG aligns with the edge

**Resynchronization**:
- Occurs during frame reception
- PS1 can be extended or PS2 shortened (up to SJW)
- Compensates for clock drift

## Baud Rate

Common baud rates:
- 125 kbps (low speed)
- 250 kbps
- 500 kbps (typical)
- 1 Mbps (high speed)

Maximum bus length depends on baud rate:
- 1 Mbps → ~40m
- 500 kbps → ~100m
- 125 kbps → ~500m

## Bus States

```
Error Active ←→ Error Passive → Bus Off
     ↑                           ↓
     └────── Recovery ───────────┘
```

- **Error Active**: TEC, REC < 128, normal operation
- **Error Passive**: TEC or REC ≥ 128, limited TX
- **Bus Off**: TEC > 255, no TX allowed

### Error State Transitions

| State | TEC | REC | Behavior |
|-------|-----|-----|----------|
| Error Active | 0-127 | 0-127 | Active error flags |
| Error Passive | 128-255 | 128-255 | Passive error flags |
| Bus Off | >255 | - | No transmission |

### Recovery from Bus-Off

1. Wait for 128 occurrences of 11 recessive bits
2. Or perform hardware/software reset
3. Counters reset to 0

## Termination

- 120 ohm at each end of bus
- Total termination: 60 ohm (parallel)
- Prevents signal reflection

## Bus Load Calculation

```
Bus Load = (Actual Frame Time / Nominal Frame Time) × 100%

Standard Frame (11-bit ID, 8 bytes):
- Nominal bits: 1 + 11 + 1 + 1 + 1 + 4 + 64 + 15 + 2 + 7 + 3 = 111 bits
- With bit stuffing: ~130 bits average

Extended Frame (29-bit ID, 8 bytes):
- Nominal bits: 1 + 29 + 1 + 1 + 1 + 4 + 64 + 15 + 2 + 7 + 3 = 135 bits
- With bit stuffing: ~160 bits average
```

### Example: 500 kbps Bus Load

```
Message: 500 messages/second, 8 bytes each, standard ID
Bits per message: ~130 bits (with stuffing)
Total bits/second: 500 × 130 = 65,000 bits
Bus load: 65,000 / 500,000 = 13%
```

**Recommended max bus load: 30-50%** to allow for:
- Error recovery
- Message retransmission
- Low priority message transmission

### 11 Recessive Bits = Bus Idle

CAN bus is considered idle after detecting 11 consecutive recessive bits:

```
Ack Delimiter (1) + EOF (7) + IFS (3) = 11 bits
```

This is why bus-off recovery requires 128 × 11 recessive bits.

## Differential Signaling Benefits

1. **Noise Immunity**: Common mode noise cancels
2. **EMI Reduction**: Opposite currents cancel magnetic fields
3. **Ground Independence**: Works across ground potential differences
