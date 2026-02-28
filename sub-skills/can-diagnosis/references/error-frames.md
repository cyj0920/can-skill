# CAN Error Frames

## Overview

Error frames are transmitted when a node detects an error on the bus.
Understanding error types helps diagnose CAN communication problems.

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

## Error Types

### 1. Bit Error

**Detection:** Transmitted bit ≠ Received bit

**Location:** Any bit except:
- Arbitration field (ID + RTR)
- ACK slot
- Passive error flag

**Causes:**
- Transceiver failure
- Bus contention
- Short circuit

**LEC Code:** 4 or 5

### 2. Stuff Error

**Detection:** 6 consecutive bits with same polarity

**Location:** Between SOF and CRC delimiter

**Causes:**
- Baud rate mismatch
- Signal integrity issues
- Synchronization failure

**LEC Code:** 1

### 3. CRC Error

**Detection:** Received CRC ≠ Calculated CRC

**Location:** CRC field

**Causes:**
- Signal integrity
- Timing errors
- Noise corruption

**LEC Code:** 6

### 4. Form Error

**Detection:** Invalid fixed-format bits

**Location:**
- CRC delimiter (should be recessive)
- ACK delimiter (should be recessive)
- EOF field (should be 7 recessive)
- Interframe space

**Causes:**
- Transmitter timing error
- Bus collision
- Noise

**LEC Code:** 2

### 5. ACK Error

**Detection:** ACK slot not dominant during transmission

**Location:** ACK field (after CRC)

**Causes:**
- No other node on bus
- All other nodes in error passive/bus-off
- RX circuitry failure on other nodes

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

**Indicates:** TX problems

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

## Error Recovery Strategies

### For ACK Errors
1. Verify other nodes are active
2. Check if other nodes can RX
3. Verify filter configuration

### For CRC Errors
1. Check signal integrity
2. Verify baud rate
3. Check for noise sources

### For Bit Errors
1. Check transceiver
2. Verify bus not shorted
3. Check for multiple transmitters

### For Stuff Errors
1. Verify baud rate match
2. Check sample point
3. Analyze signal edges

## Diagnostic Checklist

| Error Type | Primary Check | Secondary Check |
|------------|---------------|-----------------|
| ACK | Other nodes active | Transceiver |
| CRC | Signal quality | Baud rate |
| Bit | Transceiver | Bus wiring |
| Stuff | Baud rate | Timing |
| Form | Transmitter | Noise |
