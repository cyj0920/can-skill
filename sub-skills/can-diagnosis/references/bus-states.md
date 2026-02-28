# CAN Bus States

## Overview

CAN controllers have defined bus states based on error counters,
which affect communication capability.

## State Definitions

```
                ┌─────────────────────────────────────┐
                │                                     │
                ▼                                     │
          ┌──────────┐                          ┌──────────┐
          │  Error   │◄────── TEC,REC < 128 ────│  Error   │
          │  Active  │                          │  Passive │
          └──────────┘                          └──────────┘
                │                                     ▲
                │ TEC ≥ 128                           │
                ▼                                     │
          TEC ≥ 256                                  │
                │                                     │
                ▼                                     │
          ┌──────────┐                               │
          │  Bus     │                               │
          │  Off     │──────── Recovery ─────────────┘
          └──────────┘
```

## Error Active State

### Characteristics
- TEC < 128 AND REC < 128
- Normal TX and RX operation
- Error frames: 6 dominant + 8 recessive bits

### Behavior
- Can transmit and receive normally
- Error frames transmitted on error detection
- Full bus participation
- Active error flag (6 dominant bits) corrupts ongoing transmission

## Error Passive State

### Characteristics
- TEC ≥ 128 OR REC ≥ 128
- Limited TX capability
- Suspend transmission after error

### Behavior Changes

**Transmit:**
- After error, must wait 8 bits (suspend transmission)
- Error frames: 6 recessive + 8 recessive bits
- Cannot overload bus with dominant bits

**Receive:**
- Normal RX operation
- Error frames sent recessive (no effect on bus)

### What Error Passive Means

**High TEC (≥128)**: Node's transmitter is unreliable
- Errors are likely caused by this node
- Other nodes should not be affected
- Suspend transmission gives bus to other nodes

**High REC (≥128)**: Node's receiver detects many errors
- May indicate bus-wide problems
- Or this node has receive issues
- Node can still transmit normally

## Bus Off State

### Characteristics
- TEC > 255
- No TX allowed
- RX may still work (implementation dependent)

### Behavior
- Transmitter disabled
- Cannot send any frames, ACK, or error frames
- Logically disconnected from bus

### Why Bus Off Happens

TEC accumulates when TX errors occur:
- Each TX error: TEC +8
- Minimum errors to Bus Off: 32 (32 × 8 = 256 > 255)

**Note**: If only one node on bus:
- No ACK received, TEC maxes at 128
- Node enters Error Passive, not Bus Off
- Because no ACK during Error Passive doesn't increment TEC

## Error Counter Rules

### TEC Increment Rules

| Condition | TEC Change |
|-----------|------------|
| TX error (Error Active) | +8 |
| TX error (Error Passive, active error flag sent) | +8 |
| TX error (Error Passive, no active error flag) | +1 |
| Detect 14+ dominant bits after error flag | +8 |
| Detect 8 dominant bits after passive error flag | +8 |
| TX success (with ACK) | -1 (min 0) |
| Bus-off recovery | 0 |

### TEC Exceptions (No Increment)

1. **Arbitration lost**: Sending recessive, receiving dominant
2. **Error Passive + no ACK**: ACK slot recessive, no response
3. **Passive error flag**: Sending recessive, receiving dominant

### REC Increment Rules

| Condition | REC Change |
|-------|------------|
| RX error | +1 |
| RX error after dominant bit TX | +8 |
| RX success | -1 (min 0) |
| Bus-off recovery | 0 |

### Why 32 Errors Cause Bus Off

```
TEC starts at 0
Each TX error: +8
After 32 TX errors: 32 × 8 = 256 > 255 → Bus Off
```

**However**: If other messages succeed between errors:
- Each successful TX: TEC -1
- Takes more than 32 errors to reach Bus Off

**Example**: Node sends 5 messages, only ID 0x10 is disturbed:
```
Error on 0x10: TEC +8
Success on others: TEC -1 × 4 = -4
Net change: +4 per cycle
Requires 64 error cycles to Bus Off
```

## Monitoring Error Counters

```c
typedef struct {
    uint8_t tec;    // Transmit error counter
    uint8_t rec;    // Receive error counter
    uint8_t state;  // Bus state
} CAN_Status_t;

CAN_Status_t CAN_GetStatus(void)
{
    CAN_Status_t status;
    uint32_t esr = CAN->ESR;
    
    status.tec = (esr >> 16) & 0xFF;
    status.rec = (esr >> 24) & 0xFF;
    
    if (esr & CAN_ESR_BOFF) {
        status.state = CAN_STATE_BUS_OFF;
    } else if (esr & CAN_ESR_EPVF) {
        status.state = CAN_STATE_ERROR_PASSIVE;
    } else if (esr & CAN_ESR_EWGF) {
        status.state = CAN_STATE_ERROR_WARNING;
    } else {
        status.state = CAN_STATE_ERROR_ACTIVE;
    }
    
    return status;
}
```

## Warning Threshold

When TEC or REC reaches warning level (typically 96):

```c
// Enable warning interrupt
CAN->IER |= CAN_IER_EWGIE;

// In ISR
if (CAN->ESR & CAN_ESR_EWGF) {
    // Warning threshold reached
    // Log or take action
}
```

## State Transitions

| From | To | Condition |
|------|-----|-----------|
| Active | Warning | TEC or REC ≥ 96 |
| Active | Passive | TEC or REC ≥ 128 |
| Warning | Active | TEC and REC < 96 |
| Passive | Active | TEC and REC < 128 |
| Passive | Bus Off | TEC > 255 |
| Bus Off | Active | Recovery complete |

## Diagnostic Values

| TEC/REC Value | State | Action |
|---------------|-------|--------|
| 0-7 | Healthy | Normal operation |
| 8-95 | Good | Monitor |
| 96-127 | Warning | Investigate potential issues |
| 128-255 | Error Passive | Debug required |
| >255 | Bus Off | Critical fault |

## Error Counter Analysis

### High TEC, Low REC

**Indicates**: TX problem at this node

**Check**:
- Transceiver TX output
- Message format
- Bus access timing

### Low TEC, High REC

**Indicates**: RX or bus-wide problem

**Check**:
- Signal quality
- Bus termination
- Other nodes' TX quality

### Both High

**Indicates**: Bus-wide issue

**Check**:
- Baud rate mismatch
- Signal integrity
- EMI interference
- Ground problems

## Bus Off Recovery

### Automatic Recovery

```c
CAN->MCR |= CAN_MCR_ABOM;  // Enable auto bus-off management
// Controller auto-recovers after 128 × 11 recessive bits
```

### Manual Recovery

```c
void CAN_BusOff_Recovery(void)
{
    // Enter init mode
    CAN->MCR |= CAN_MCR_INRQ;
    while(!(CAN->MSR & CAN_MSR_INAK));
    
    // Reset error counters
    // (Reading ESR clears pending errors)
    (void)CAN->ESR;
    
    // Exit init mode
    CAN->MCR &= ~CAN_MCR_INRQ;
    while(CAN->MSR & CAN_MSR_INAK);
}
```

For detailed recovery strategies, see `busoff-recovery.md`.

## Best Practices

1. **Monitor counters regularly**
   - Poll ESR register periodically
   - Set threshold interrupts

2. **Log state transitions**
   - Track when errors increase
   - Correlate with system events

3. **Implement recovery**
   - Enable ABOM for auto recovery
   - Add manual recovery as backup

4. **Root cause analysis**
   - High TEC: TX problems
   - High REC: RX/bus problems
   - Both high: Bus-wide issue

5. **DTC logging**
   - Record Bus Off events
   - Track recovery attempts
   - Correlate with other faults