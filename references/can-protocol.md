# CAN Protocol Basics

## Overview

CAN (Controller Area Network) is a serial communication protocol designed for
robust communication in automotive and industrial environments.

## Key Features

- Multi-master architecture
- Message-based communication
- Built-in error detection
- Automatic arbitration
- High noise immunity

## Frame Types

| Frame Type | Purpose |
|------------|---------|
| Data Frame | Transmit data |
| Remote Frame | Request data |
| Error Frame | Signal error |
| Overload Frame | Delay next frame |

## Data Frame Structure

```
+------+--------+------+----------+------+-----+-----+------+
| SOF  | Arbit. | Ctrl | Data     | CRC  | ACK | EOF | IFS  |
| 1bit | 12-32b | 6bit | 0-8bytes | 15b  | 2b  | 7b  | 3b   |
+------+--------+------+----------+------+-----+-----+------+

SOF: Start of Frame (dominant)
Arbitration: Identifier + RTR/SRR bits
Control: IDE + DLC
Data: 0-8 bytes payload
CRC: Cyclic Redundancy Check
ACK: Acknowledge slot
EOF: End of Frame
IFS: Interframe Space
```

## Bit Encoding

- **Dominant**: Logical 0, drives bus to ~0V differential
- **Recessive**: Logical 1, bus at ~2.5V differential (idle state)
- Dominant overrides recessive (wired-AND)

## Arbitration

- Non-destructive bitwise arbitration
- Lower ID = Higher priority
- ID 0x000 has highest priority
- Winner continues transmission, losers retry

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

**Recommended max bus load: 70%** to allow for:
- Error recovery
- Message retransmission
- Network expansion

## Physical Layer Voltage Levels

| State | CAN_H | CAN_L | Differential |
|-------|-------|-------|--------------|
| Recessive (idle) | 2.5V | 2.5V | 0V |
| Dominant (active) | 3.5V | 1.5V | 2V |

**Voltage Tolerance**:
- V_diff (dominant): ≥ 1.5V
- V_diff (recessive): < 0.5V
- Common mode range: -2V to +7V

## Differential Signaling Benefits

1. **Noise Immunity**: Common mode noise cancels
2. **EMI Reduction**: Opposite currents cancel magnetic fields
3. **Ground Independence**: Works across ground potential differences