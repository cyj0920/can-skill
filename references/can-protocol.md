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

## Termination

- 120 ohm at each end of bus
- Total termination: 60 ohm (parallel)
- Prevents signal reflection
