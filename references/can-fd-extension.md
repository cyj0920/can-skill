# CAN-FD Extension

## Overview

CAN-FD (Flexible Data-rate) extends classical CAN with:
- Larger payload (up to 64 bytes vs 8 bytes)
- Higher bit rate during data phase
- Backward compatible with CAN 2.0

## Frame Format Differences

| Feature | CAN 2.0 | CAN-FD |
|---------|---------|--------|
| Data length | 0-8 bytes | 0-64 bytes |
| Bit rate | Fixed | Switchable |
| DLC coding | 0-8 | 0-8, 12, 16, 20, 24, 32, 48, 64 |
| CRC size | 15-bit | 17/21-bit |

## Frame Structure

```
CAN-FD Frame:
+-----+--------+------+----+----------+------+-----+-----+
| SOF | Arbit. | FDF  | ...| Data     | CRC  | ACK | EOF |
|     |        | BRS  |    | 0-64B    | 17/21b|     |     |
+-----+--------+------+----+----------+------+-----+-----+

FDF: FD Format (replaces RTR in CAN-FD)
BRS: Bit Rate Switch
ESI: Error State Indicator
```

## Bit Rate Switching

CAN-FD can use two bit rates:
1. **Arbitration phase**: Standard baud rate (e.g., 500 kbps)
2. **Data phase**: Higher baud rate (e.g., 2 Mbps)

```
     ┌─ Arbitration ─┐ ┌─── Data Phase ───┐
     │  (500 kbps)   │ │    (2 Mbps)      │
─────┴───────────────┴─┴──────────────────┴──────
     ^               ^
   SOF              BRS
```

## DLC Coding

| DLC | Data Length |
|-----|-------------|
| 0-8 | 0-8 bytes |
| 9 | 12 bytes |
| 10 | 16 bytes |
| 11 | 20 bytes |
| 12 | 24 bytes |
| 13 | 32 bytes |
| 14 | 48 bytes |
| 15 | 64 bytes |

## CRC Selection

| Data Length | CRC Size | Polynomial |
|-------------|----------|------------|
| 0-16 bytes | 17-bit | 0x1B595 |
| 17-64 bytes | 21-bit | 0x17565B |

## Compatibility

**CAN-FD controllers can:**
- Receive CAN 2.0 frames
- Transmit CAN 2.0 frames (if configured)

**CAN 2.0 controllers:**
- Will ignore CAN-FD frames (error frames if strict)
- Cannot receive CAN-FD data

## Migration Considerations

When upgrading from CAN to CAN-FD:
1. Replace transceivers (some old ones may not support FD)
2. Update firmware for FD-capable controller
3. Configure FD-specific timing parameters
4. Test backward compatibility if mixed network
