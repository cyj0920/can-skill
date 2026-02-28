# Common MCU CAN Controllers

## Overview

Most MCUs use similar CAN controller architecture, but with different
register names and memory organization.

## STM32 CAN Controllers

### Supported Series
| Series | CAN Module | CAN-FD Support |
|--------|------------|----------------|
| F1 | bxCAN | No |
| F4 | bxCAN | No |
| F7 | bxCAN | No |
| H7 | FDCAN | Yes |
| G4 | FDCAN | Yes |

### bxCAN Features (F1/F4/F7)
- Up to 3 CAN controllers (F4/F7)
- 28 filter banks (shared)
- 3 TX mailboxes
- 2 RX FIFOs (3 messages each)
- 256-byte SRAM

### FDCAN Features (H7/G4)
- CAN 2.0 and CAN-FD
- Dedicated message RAM
- Extended filter support
- TX event FIFO
- Timestamp support

## NXP CAN Controllers

### S32K Series
| Feature | S32K1xx | S32K3xx |
|---------|---------|---------|
| Module | FlexCAN | FlexCAN |
| CAN-FD | Yes | Yes |
| Message Buffers | Up to 96 | Up to 96 |
| RX FIFO | Yes | Yes |

### LPC Series
- C_CAN module
- 32 message objects
- Basic CAN functionality

### FlexCAN Features
- Individual message buffers
- RX FIFO mode
- RX mask registers
- Self-reception support

## Infineon CAN Controllers

### Aurix TC3xx
| Feature | MultiCAN+ |
|---------|-----------|
| Controllers | Up to 4 |
| Nodes per controller | Up to 8 |
| Message objects | Up to 256 |
| CAN-FD | Yes |

### TC2xx
- MultiCAN module
- Similar to TC3xx but fewer nodes

## Texas Instruments

### C2000 Series
- DCAN module
- 32 message objects
- Basic CAN functionality

## Common Architecture

Despite different naming, most CAN controllers share:

```
┌─────────────────────────────────┐
│          CAN Controller         │
├─────────────────────────────────┤
│ Control Registers               │
│  - MCR (Master Control)         │
│  - MSR (Master Status)          │
│  - BTR (Bit Timing)             │
├─────────────────────────────────┤
│ TX Path                         │
│  - TX Mailbox/Buffers           │
│  - TX Request Register          │
├─────────────────────────────────┤
│ RX Path                         │
│  - RX FIFO/Buffers              │
│  - RX Message Object            │
├─────────────────────────────────┤
│ Filter                          │
│  - Acceptance Filters           │
│  - Mask Registers               │
├─────────────────────────────────┤
│ Error Handling                  │
│  - Error Counter (TEC/REC)      │
│  - Error Status Register        │
└─────────────────────────────────┘
```

## Transceiver Interface

All MCU CAN controllers connect to external transceiver:

```
MCU                    Transceiver
┌────┐                 ┌──────────┐    CAN Bus
│CAN ├── CANTX ────────│TXD   CANH├──────┬─────
│    │                 │          │      │
│    ├── CANRX ────────│RXD   CANL├──────┴─────
└────┘                 └──────────┘   120Ω
```

Common transceivers:
- TJA1050 (classic, 1 Mbps max)
- TJA1042/TJA1043 (modern, low power)
- SN65HVD230 (3.3V compatible)
