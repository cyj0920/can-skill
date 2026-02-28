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

## Detailed Troubleshooting Guide

### Error Frame Analysis

| Error Type | Primary Check | Secondary Check | Tools |
|------------|---------------|-----------------|-------|
| ACK | Other nodes active | Transceiver RX | Multimeter |
| CRC | Signal quality | Baud rate | Oscilloscope |
| Bit | Transceiver | Bus wiring | Oscilloscope |
| Stuff | Baud rate | Timing/SJW | Logic analyzer |
| Form | Transmitter | Noise | Scope + trigger |

### ACK Error Solutions

1. **Verify other nodes active**
   - Check power to all nodes
   - Verify transceiver enable pins
   - Confirm software initialization

2. **Check filter configuration**
   - Other nodes may be rejecting messages
   - Verify ID acceptance filters

3. **Transceiver diagnostics**
   - Measure CAN_H/L idle voltages
   - Check TXD/RXD connections
   - Verify transceiver not in standby

### CRC Error Solutions

1. **Signal integrity check**
   - Use oscilloscope on CAN_H/L
   - Check for ringing, overshoot
   - Verify edge symmetry

2. **Common causes:**
   - Missing termination
   - Long stubs
   - EMI interference
   - Poor cable quality

3. **Solutions:**
   - Add proper termination (120Ω each end)
   - Shorten or remove stubs
   - Use shielded cable
   - Add common mode choke

### Bit Error Solutions

1. **Transceiver check**
   - Verify VCC (typically 5V)
   - Check thermal status
   - Replace if suspected

2. **Bus wiring check**
   - Check for shorts (CAN_H to CAN_L)
   - Check for opens
   - Verify polarity (H to H, L to L)

3. **Ground reference**
   - Ensure all nodes share ground
   - Check for ground loops
   - Measure voltage between node grounds

### Stuff Error Solutions

1. **Baud rate verification**
   - Check all nodes same baud rate
   - Verify clock frequency
   - Calculate actual vs nominal rate
   - **Tolerance: < 1.5%**

2. **SJW configuration**
   - Increase SJW if possible
   - Typical: 1-2 Tq
   - Allows better resynchronization

3. **Clock source quality**
   - Use crystal oscillator for CAN
   - Avoid internal RC oscillators
   - Check oscillator stability

### Form Error Solutions

1. **Timing verification**
   - Check sample point position
   - Verify propagation delay settings
   - Consider cable length effects

2. **Noise investigation**
   - Use shielded cables
   - Check for EMI sources
   - Improve grounding

## Diagnostic Checklist

### Quick Physical Layer Check

```
□ Power off: Measure CAN_H to CAN_L = ~60Ω
□ Power on: CAN_H idle = ~2.5V
□ Power on: CAN_L idle = ~2.5V
□ Power on: Differential idle = ~0V
□ During TX: CAN_H = ~3.5V, CAN_L = ~1.5V
```

### Error Counter Monitoring

```
□ Read TEC and REC every 100ms
□ Log LEC values
□ Track state transitions
□ Monitor bus-off recovery
```

### Baud Rate Verification

```
□ Calculate actual baud rate
□ Check all nodes match
□ Verify clock accuracy
□ Confirm tolerance < 1.5%
```

## Error Recovery Strategies

### For ACK Errors
1. Verify other nodes are active
2. Check if other nodes can RX
3. Verify filter configuration
4. Check transceiver enable pins

### For CRC Errors
1. Check signal integrity with scope
2. Verify baud rate match
3. Check for noise sources
4. Verify termination

### For Bit Errors
1. Check transceiver
2. Verify bus not shorted
3. Check for multiple transmitters
4. Verify ground connections

### For Stuff Errors
1. Verify baud rate match (all nodes)
2. Check sample point configuration
3. Analyze signal edges
4. Check clock source quality

### For Form Errors
1. Check transmitter timing
2. Verify bus length vs baud rate
3. Check for EMI/EMC issues
4. Investigate signal reflections

## Common Fault Signatures

| Symptom | Likely Error | Root Cause |
|---------|--------------|------------|
| Immediate bus-off | Bit Error | Short circuit, bad transceiver |
| Intermittent errors | CRC/Stuff | EMI, loose connection |
| Errors after warm-up | Various | Thermal drift, component failure |
| Single node affected | ACK | Transceiver, software config |
| All nodes affected | Bit/Stuff | Bus wiring, termination |