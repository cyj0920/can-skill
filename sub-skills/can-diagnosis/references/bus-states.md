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
          │  Error   │◄────── REC < 128 ────────│  Error   │
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

## Error Passive State

### Characteristics
- TEC ≥ 128 OR REC ≥ 128
- Limited TX capability
- Suspend transmission after error

### Behavior Changes

**Transmit:**
- After error, must wait 8 bits (suspend)
- Error frames: 6 recessive + 8 recessive bits
- Cannot overload with dominant bits

**Receive:**
- Normal RX operation
- Error frames sent recessive

### Detection
```c
bool is_error_passive = (CAN->ESR & CAN_ESR_EPVF) != 0;
```

## Bus Off State

### Characteristics
- TEC > 255
- No TX allowed
- RX may still work

### Behavior
- Transmitter disabled
- Cannot send any frames
- May still receive

### Recovery

**Automatic (ABOM enabled):**
```c
CAN->MCR |= CAN_MCR_ABOM;  // Enable auto bus-off management
// Controller auto-recovers after 128 × 11 recessive bits
```

**Manual:**
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

## Error Counters

### Transmit Error Counter (TEC)

| Event | TEC Change |
|-------|------------|
| TX error | +8 |
| TX success | -1 (min 0) |
| TX error in passive state | +8 (if active error flag) |
| TX error in passive state | +1 (if not) |

### Receive Error Counter (REC)

| Event | REC Change |
|-------|------------|
| RX error | +1 |
| RX success | -1 (min 0) |
| RX error after TX error | +8 |
| Bus-off recovery | Reset to 0 |

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
| Bus Off | Active | 128 × 11 recessive bits |

## Diagnostic Values

| TEC/REC Value | State | Action |
|---------------|-------|--------|
| 0-7 | Healthy | Normal |
| 8-95 | Good | Monitor |
| 96-127 | Warning | Investigate |
| 128-255 | Error Passive | Debug required |
| >255 | Bus Off | Critical fault |

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
