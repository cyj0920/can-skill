# CAN-FD Extension Guide

## Overview

CAN-FD (Flexible Data-rate) extends the Classical CAN protocol with:
- Larger payload (up to 64 bytes)
- Higher data phase bit rate (up to 5 Mbps)
- Improved CRC algorithm

## ISO vs Non-ISO CAN-FD

**Critical**: Two incompatible CAN-FD versions exist!

| Feature | ISO CAN-FD | Non-ISO CAN-FD |
|---------|------------|----------------|
| Standard | ISO 11898-1:2015 | Bosch 2012 spec |
| CRC | Improved with SBC | Original |
| Compatibility | Industry standard | Legacy only |
| Error detection | Enhanced | Weaker |

**Action**: Always use ISO CAN-FD for new designs. Check transceiver
and controller datasheets for "ISO CAN-FD" certification.

## CAN-FD vs Classical CAN

| Feature | Classical CAN | CAN-FD |
|---------|---------------|--------|
| Max data bytes | 8 | 64 |
| Max bit rate | 1 Mbps | 5 Mbps (data phase) |
| Remote frame | Supported | Not supported |
| CRC bits | 15 | 17 or 21 |
| Bit stuffing | Dynamic | Fixed in CRC |

## CAN-FD Frame Format

```
Classical CAN:  SOF | ID | RTR | IDE | r0 | DLC | DATA | CRC | ACK | EOF
                  1  | 11 |  1  |  1  | 1  |  4  | 0-8B | 15  |  2  |  7

CAN-FD:         SOF | ID | RRS | IDE | FDF| res| BRS | ESI | DLC | DATA | SBC | CRC | ACK | EOF
                  1  | 11 |  1  |  1  | 1  | 1  |  1  |  1  |  4  |0-64B |  4  |17/21|  2  |  7
```

### New Fields

| Field | Bits | Purpose |
|-------|------|---------|
| FDF | 1 | FD Format indicator (recessive = CAN-FD) |
| res | 1 | Reserved bit |
| BRS | 1 | Bit Rate Switch (recessive = switch rate) |
| ESI | 1 | Error State Indicator |
| SBC | 4 | Stuff Bit Count (gray code + parity) |

## DLC Mapping

| DLC | Classical CAN | CAN-FD |
|-----|---------------|--------|
| 0-8 | 0-8 bytes | 0-8 bytes |
| 9 | Invalid | 12 bytes |
| 10 | Invalid | 16 bytes |
| 11 | Invalid | 20 bytes |
| 12 | Invalid | 24 bytes |
| 13 | Invalid | 32 bytes |
| 14 | Invalid | 48 bytes |
| 15 | Invalid | 64 bytes |

## Dual Bit Rate

CAN-FD uses two bit rates:

1. **Arbitration Phase** (SOF to BRS)
   - Max 1 Mbps (same as Classical CAN)
   - Allows long bus lengths
   - All nodes must participate

2. **Data Phase** (BRS to ACK delimiter)
   - Up to 5 Mbps typical
   - Limited by transceiver and topology
   - Only winner transmits

```
Arbitration Phase          Data Phase
      ↓                        ↓
├────────────────┬──────────────────────┤
│  500 kbps      │     2 Mbps           │
│  (up to 1Mbps) │   (up to 5Mbps)      │
└────────────────┴──────────────────────┘
                 ↑
             BRS bit
```

## Data Phase Bit Rate Limits

| Transceiver Type | Max Data Rate | Notes |
|------------------|---------------|-------|
| Standard (ISO 11898-2) | 1 Mbps | Not CAN-FD certified |
| CAN-FD Basic | 2 Mbps | -40°C to +125°C |
| CAN-FD Enhanced | 5 Mbps | Improved symmetry |
| ECU Flashing | up to 12 Mbps | Limited temp range |

**CiA 601-3 Recommendation**:
- Multi-drop bus: max 2 Mbps @ -40°C to +125°C
- Bus-line topology with short stubs: higher rates possible

## Efficiency Comparison

| Payload | Classical CAN | CAN-FD (same rate) | CAN-FD (BRS) |
|---------|---------------|--------------------|--------------|
| 1 byte | 56% | 44% | 44% |
| 4 bytes | 72% | 59% | 72% |
| 8 bytes | 79% | 66% | 84% |
| 16 bytes | N/A | 75% | 90% |
| 64 bytes | N/A | 87% | 96% |

**Note**: CAN-FD efficiency exceeds Classical CAN only after 8+ bytes.

## CRC Algorithm

CAN-FD uses improved CRC with two sizes:

| Data Length | CRC Size | Polynomial |
|-------------|----------|------------|
| 0-16 bytes | 17 bits | 0x1B595 |
| 17-64 bytes | 21 bits | 0x1B2B9 |

**Fixed Stuff Bits in CRC**: 4 fixed stuff bits improve reliability

## Mixed Network Configuration

CAN-FD controllers can coexist with Classical CAN nodes:

### Scenario 1: All CAN-FD Nodes
- Free mix of Classical CAN and CAN-FD frames
- Full CAN-FD benefits available

### Scenario 2: Mixed Classical CAN and CAN-FD
```
Configuration:
- CAN-FD controllers must use Classical CAN format
- Or disable Classical CAN nodes during CAN-FD communication
- Classic nodes will error on CAN-FD frames (FDF bit = 1)
```

### Migration Strategy

```
Phase 1: Install CAN-FD capable ECUs (operating in Classic mode)
Phase 2: Upgrade network to CAN-FD (disable or replace Classic nodes)
Phase 3: Enable CAN-FD features on all nodes
```

## Sample Point for Data Phase

**Recommendation**: Set data phase sample point differently

| Phase | Sample Point | Reason |
|-------|--------------|--------|
| Arbitration | 87.5% | Standard, allows long bus |
| Data | 62.5%-75% | Faster edges, shorter propagation |

## Configuration Example (STM32 FDCAN)

```c
// Arbitration phase: 500 kbps, 87.5% sample point
// Data phase: 2 Mbps, 62.5% sample point
// APB clock: 80 MHz

// Nominal timing (arbitration)
// 500 kbps = 80MHz / 40 / 4 (Prescaler=40, TotalTq=40)
uint32_t nominal_prescaler = 40;
uint32_t nominal_time_seg1 = 34;  // Prop + PS1
uint32_t nominal_time_seg2 = 5;   // PS2
// Sample point = (1 + 34) / 40 = 87.5%

// Data phase timing (2 Mbps)
// 2 Mbps = 80MHz / 10 / 4 (Prescaler=10, TotalTq=4)
uint32_t data_prescaler = 10;
uint32_t data_time_seg1 = 2;
uint32_t data_time_seg2 = 1;
// Sample point = (1 + 2) / 4 = 75%

// Configure FDCAN
FDCAN_Config->NominalPrescaler = nominal_prescaler;
FDCAN_Config->NominalTimeSeg1 = nominal_time_seg1;
FDCAN_Config->NominalTimeSeg2 = nominal_time_seg2;
FDCAN_Config->DataPrescaler = data_prescaler;
FDCAN_Config->DataTimeSeg1 = data_time_seg1;
FDCAN_Config->DataTimeSeg2 = data_time_seg2;
FDCAN_Config->EnableBitRateSwitching = 1;
```

## Transceiver Selection

| Requirement | Recommended Transceiver |
|-------------|------------------------|
| Basic CAN-FD | TJA1043, TCAN1043 |
| High speed (5 Mbps) | TJA1055, TCAN1055 |
| With protection | TJA1145 (FD + SBC) |
| Industrial | ISO1050 (isolated) |

## Key Differences Summary

1. **No Remote Frames** in CAN-FD - RRS always dominant
2. **BRS enables higher speed** during data transmission
3. **ESI indicates error state** of transmitter
4. **Larger CRC** for better error detection
5. **Fixed stuff bits** in CRC field for reliability