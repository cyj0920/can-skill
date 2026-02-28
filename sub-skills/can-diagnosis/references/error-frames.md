# CAN Error Frames

## Overview

Error frames are transmitted when a node detects an error on the bus.
Understanding error types helps diagnose CAN communication problems.

**Important**: Over 80% of CAN bus issues originate from the physical layer.
Always check hardware first before investigating software configuration.

## Error Frame Structure

```
Error Frame:
┌─────────────────┬──────────────────┐
│  Error Flag     │  Error Delimiter │
│  6 bits         │  8 bits          │
└─────────────────┴──────────────────┘

Active Error Flag: 6 dominant bits
Passive Error Flag: 6 recessive bits
Error Delimiter: 8 recessive bits
```

## Error Flag Overlap

When multiple nodes detect errors simultaneously, their error flags overlap
on the bus due to the wired-AND nature of CAN:

```
Node A (TX):  ──────┌──┬──┬──┬──┬──┬──┬─────────────────┐
                    │E1│E2│E3│E4│E5│E6│                 │
                    │  │  │  │  │  │  │                 │
Node B (RX):  ──────┼──┼──┼──┼──┼──┼──┼──┬──┬──┬──┬──┬──┬──┤
                    │  │  │  │  │  │  │E1│E2│E3│E4│E5│E6│  │
                    │  │  │  │  │  │  │  │  │  │  │  │  │  │
Bus:          ──────┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴────
                    ├─────────────┤├─────────────┤
                    Node A flag    Node B flag
                    ├────────────────────────────┤
                         Overlap: 6-12 bits
```

### Overlap Characteristics

- **Active + Active**: 6-12 dominant bits on bus
- **Active + Passive**: 6 dominant bits (passive overwritten)
- **Passive + Passive**: 12 recessive bits

### Why Overlap Occurs

1. Node A detects error first, sends error flag
2. Error flag violates bit stuffing (6 dominant bits)
3. Node B detects stuffing error
4. Node B sends its error flag
5. Flags overlap on bus

## TX vs RX Node Error Detection

**Key Insight**: Transmitter and receiver may detect DIFFERENT error types
at the SAME moment!

### Example Scenario

```
Timeline:
T1: TX Node sends dominant bit
    TX monitors bus, sees recessive → BIT ERROR detected!
    TX starts sending active error flag (6 dominant bits)

T2: RX Node sees 6 dominant bits
    RX detects STUFF ERROR (6 same bits violated stuffing rule)
    RX sends its own active error flag

Result:
- TX Node: Bit Error
- RX Node: Stuff Error
- Both occur at the same time!
```

### Why Different Errors?

| Role | Can Detect | Cannot Detect |
|------|------------|---------------|
| TX Node | Bit Error (knows what it sent) | - |
| RX Node | Stuff Error, CRC Error, Form Error | Bit Error (doesn't know TX intent) |

### Error Detection Timing

| Error Type | When Detected | Error Frame Sent |
|------------|---------------|------------------|
| Bit Error | Current bit | Next bit |
| Stuff Error | 6th same bit | Next bit |
| CRC Error | After CRC field | After ACK delimiter |
| Form Error | Invalid fixed bit | Next bit |
| ACK Error | ACK slot | Next bit |

## Error Types

### 1. Bit Error

**Detection:** Transmitted bit ≠ Received bit

**Location:** Any bit except:
- Arbitration field (ID + RTR) - arbitration loss not error
- ACK slot - no ACK is normal
- Passive error flag

**TX Node Exclusive**: Only transmitter can detect bit errors
because only TX knows what it intended to send.

**Causes:**
- Transceiver failure
- Bus contention
- Short circuit
- Signal integrity issues

**LEC Code:** 4 or 5

### 2. Stuff Error

**Detection:** 6 consecutive bits with same polarity

**Location:** Between SOF and CRC delimiter

**Both TX and RX** can detect stuff errors.

**Causes:**
- Baud rate mismatch
- Signal integrity issues
- Synchronization failure
- Bit errors from other node

**LEC Code:** 1

### 3. CRC Error

**Detection:** Received CRC ≠ Calculated CRC

**Location:** CRC field

**RX Node Only**: Only receivers calculate and verify CRC.

**Causes:**
- Signal integrity
- Timing errors
- Noise corruption
- Bit errors earlier in frame

**LEC Code:** 6

### 4. Form Error

**Detection:** Invalid fixed-format bits

**Location:**
- CRC delimiter (should be recessive)
- ACK delimiter (should be recessive)
- EOF field (should be 7 recessive)
- Interframe space

**Both TX and RX** can detect form errors.

**Exceptions** (not form errors):
- EOF last bit dominant (acceptable)
- DLC 9-15 in data frame (acceptable)

**Causes:**
- Transmitter timing error
- Bus collision
- Noise

**LEC Code:** 2

### 5. ACK Error

**Detection:** ACK slot not dominant during transmission

**Location:** ACK field (after CRC)

**TX Node Only**: Only transmitter checks for ACK.

**Causes:**
- No other node on bus
- All other nodes in error passive/bus-off
- RX circuitry failure on other nodes
- All other nodes filtering out this ID

**LEC Code:** 3

## Last Error Code (LEC)

| LEC | Error Type | Description |
|-----|------------|-------------|
| 0 | No Error | No error since last read |
| 1 | Stuff Error | Bit stuffing violation |
| 2 | Form Error | Fixed format field error |
| 3 | ACK Error | No acknowledgment |
| 4 | Bit Recessive Error | TX recessive ≠ RX |
| 5 | Bit Dominant Error | TX dominant ≠ RX |
| 6 | CRC Error | CRC mismatch |
| 7 | No Change | Controlled by software |

## Active vs Passive Error Flags

### Active Error Flag (Error Active State)

```
┌──────────────────────────────────────┐
│ D D D D D D │ R R R R R R R R │
│ 6 dominant  │  8 recessive    │
│ Error Flag  │  Delimiter      │
└──────────────────────────────────────┘

Effect: Corrupts current transmission, all nodes see error
```

### Passive Error Flag (Error Passive State)

```
┌──────────────────────────────────────┐
│ R R R R R R │ R R R R R R R R │
│ 6 recessive │  8 recessive    │
│ Error Flag  │  Delimiter      │
└──────────────────────────────────────┘

Effect: May not corrupt bus (dominant bits from other nodes win)
```

### Error Flag Behavior

| State | Flag Type | Bus Impact | After Error |
|-------|-----------|------------|-------------|
| Error Active | 6 dominant | Corrupts transmission | Immediate retry |
| Error Passive | 6 recessive | May not affect bus | Suspend 8 bits |

## Reading Error Status

```c
typedef struct {
    uint8_t tec;           // TX error counter
    uint8_t rec;           // RX error counter
    uint8_t lec;           // Last error code
    bool bus_off;          // Bus-off flag
    bool error_passive;    // Error passive flag
    bool error_warning;    // Error warning flag
} CAN_Error_t;

CAN_Error_t CAN_GetErrorStatus(void)
{
    CAN_Error_t err;
    uint32_t esr = CAN->ESR;
    
    err.tec = (esr >> 16) & 0xFF;
    err.rec = (esr >> 24) & 0xFF;
    err.lec = (esr >> 4) & 0x07;
    
    err.bus_off = (esr & CAN_ESR_BOFF) != 0;
    err.error_passive = (esr & CAN_ESR_EPVF) != 0;
    err.error_warning = (esr & CAN_ESR_EWGF) != 0;
    
    return err;
}
```

## Error Counter Effects

### TEC Increment (TX Error)

| Condition | Increment |
|-----------|-----------|
| Active state, error detected | +8 |
| Passive state, first TX error | +8 |
| Passive state, subsequent | +1 |
| Successful TX | -1 (min 0) |

### REC Increment (RX Error)

| Condition | Increment |
|-----------|-----------|
| Error during RX | +1 |
| Error during dominant bit TX | +8 |
| Successful RX | -1 (min 0) |

## Error Pattern Analysis

### High TEC, Low REC

**Indicates:** TX problems at this node

**Check:**
- TX mailbox status
- Transceiver TX output
- Message format correctness

### Low TEC, High REC

**Indicates:** RX/bus problems

**Check:**
- Bus signal quality
- Termination
- Other node TX quality

### Both High

**Indicates:** Bus-wide issues

**Check:**
- Baud rate mismatch
- Signal integrity
- Ground problems
- EMI interference

## Common Fault Signatures

| Symptom | Likely Error | Root Cause |
|---------|--------------|------------|
| Immediate bus-off | Bit Error | Short circuit, bad transceiver |
| Intermittent errors | CRC/Stuff | EMI, loose connection |
| Errors after warm-up | Various | Thermal drift, component failure |
| Single node affected | ACK | Transceiver, software config |
| All nodes affected | Bit/Stuff | Bus wiring, termination |

## Diagnostic Checklist

### Quick Physical Layer Check

```
□ Power off: Measure CAN_H to CAN_L = ~60Ω
□ Power on: CAN_H idle = ~2.5V
□ Power on: CAN_L idle = ~2.5V
□ Power on: Differential idle = ~0V
□ During TX: CAN_H = ~3.5V, CAN_L = ~1.5V
```

### Error Analysis

```
□ Log LEC values with timestamps
□ Correlate errors with system events
□ Check if errors are TX or RX related
□ Monitor TEC/REC trends
□ Identify patterns (periodic, random, triggered)
```
