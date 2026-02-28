# CAN Bus-Off Recovery

## Overview

When a CAN node's Transmit Error Counter (TEC) exceeds 255, it enters the
Bus-Off state and can no longer participate in bus communication.
Recovery strategies are essential for reliable system operation.

## Why 128 × 11 Recessive Bits?

CAN specification requires 128 occurrences of 11 consecutive recessive bits
for automatic Bus-Off recovery. Here's why:

### What is 11 Recessive Bits?

After each CAN frame, the bus shows 11 recessive bits:

```
Ack Delimiter (1) + EOF (7) + Inter-Frame Space (3) = 11 bits
```

```
Frame Data... │ ACK Delim │    EOF    │  IFS  │ Next Frame
              │     1b    │    7b     │  3b   │
              └───────────┴───────────┴───────┘
                          11 recessive bits
```

### Bus Idle Detection

A node can start transmitting when it detects 11 consecutive recessive bits:
- This represents minimum bus idle time
- Faster detection = faster bus access
- 11 bits ensures proper frame separation

### Why 128 Times?

- Guarantees sufficient time for fault conditions to clear
- Provides recovery opportunity for transient faults
- Prevents rapid Bus-Off/Recovery cycling
- Total time: 128 × 11 bits = 1408 bit times

At 500 kbps:
```
Recovery time = 1408 / 500000 = 2.8 ms
```

## Recovery Mechanisms

### Automatic Recovery (ABOM)

Most CAN controllers support Automatic Bus-Off Management:

```c
// STM32 example
CAN->MCR |= CAN_MCR_ABOM;  // Enable automatic recovery

// Node will automatically:
// 1. Monitor for 128 × 11 recessive bits
// 2. Reset TEC to 0
// 3. Return to Error Active state
```

### Manual Recovery

When ABOM is disabled or faster recovery is needed:

```c
void CAN_Manual_Recovery(void)
{
    // Step 1: Enter initialization mode
    CAN->MCR |= CAN_MCR_INRQ;
    while(!(CAN->MSR & CAN_MSR_INAK));
    
    // Step 2: Clear error state
    (void)CAN->ESR;  // Read to clear
    
    // Step 3: Exit initialization mode
    CAN->MCR &= ~CAN_MCR_INRQ;
    while(CAN->MSR & CAN_MSR_INAK);
    
    // TEC is now 0, node is Error Active
}
```

## Fast/Slow Recovery Strategy

### Recovery Levels

Automotive applications often implement tiered recovery:

```
Level 1: Fast Recovery (Quick attempt)
    ↓ (failed after 5 attempts)
Level 2: Slow Recovery (Extended wait)
```

### Typical Implementation

| Recovery Type | Wait Time | Max Attempts | Action |
|---------------|-----------|--------------|--------|
| Fast Recovery | 10-100ms | 5-32 | Quick retry |
| Slow Recovery | 1-60s | Unlimited | Extended wait |

### State Machine

```
┌─────────────────────────────────────────────────────┐
│                                                     │
▼                                                     │
┌───────────┐    Bus-Off     ┌──────────────┐        │
│  Normal   │───────────────►│ Fast Recovery│        │
│ Operation │                │   (10ms)     │        │
└───────────┘                └──────┬───────┘        │
      ▲                             │                 │
      │                    Failed 5 times            │
      │                             ▼                 │
      │                     ┌──────────────┐        │
      │                     │ Slow Recovery│        │
      │                     │   (1-60s)    │        │
      │                     └──────┬───────┘        │
      │                            │                 │
      │                   Recovery │                 │
      └────────────────────────────┘                 │
                                                     │
                                          Success ───┘
```

### Code Example

```c
#define FAST_RECOVERY_MS    10
#define SLOW_RECOVERY_MS    1000
#define FAST_RECOVERY_MAX   5

typedef struct {
    uint8_t fast_count;
    uint8_t in_slow_recovery;
    uint32_t last_recovery_ms;
} BusOff_Recovery_t;

void BusOff_Recovery_Handler(BusOff_Recovery_t *ctx)
{
    uint32_t now = HAL_GetTick();
    uint32_t wait_time;
    
    // Check if still in Bus-Off
    if (!(CAN->ESR & CAN_ESR_BOFF)) {
        // Recovered successfully
        ctx->fast_count = 0;
        ctx->in_slow_recovery = 0;
        return;
    }
    
    // Determine wait time
    if (ctx->fast_count < FAST_RECOVERY_MAX) {
        wait_time = FAST_RECOVERY_MS;
        ctx->fast_count++;
    } else {
        wait_time = SLOW_RECOVERY_MS;
        ctx->in_slow_recovery = 1;
    }
    
    // Check if enough time has passed
    if (now - ctx->last_recovery_ms >= wait_time) {
        CAN_Manual_Recovery();
        ctx->last_recovery_ms = now;
    }
}
```

## DTC (Diagnostic Trouble Code) Strategy

### When to Record Bus-Off DTC

1. **After Fast Recovery Exhausted**: 
   - 5-32 consecutive Bus-Off events
   - Transition to Slow Recovery

2. **During Slow Recovery**:
   - Continuous failure indicates persistent fault

### DTC Configuration Example

| Parameter | Value | Purpose |
|-----------|-------|---------|
| Detection Period | 10ms | How often to check Bus-Off status |
| Fast Recovery Count | 32 | Attempts before slow recovery |
| Fast Recovery Time | 10ms | Wait between fast attempts |
| Slow Recovery Time | 60s | Wait between slow attempts |
| DTC Trigger | Slow Recovery Entry | When to record fault |

### Implementation

```c
void BusOff_DTC_Handler(void)
{
    static uint8_t busoff_count = 0;
    static uint32_t last_check = 0;
    uint32_t now = HAL_GetTick();
    
    // Check every 10ms
    if (now - last_check < 10) return;
    last_check = now;
    
    if (CAN->ESR & CAN_ESR_BOFF) {
        busoff_count++;
        
        // Attempt recovery
        CAN_Manual_Recovery();
        
        // Record DTC after 32 consecutive Bus-Offs
        if (busoff_count >= 32) {
            DTC_Set(DTC_CAN_BUSOFF);
            busoff_count = 0;
        }
    } else {
        // Successful recovery, reset counter
        busoff_count = 0;
    }
}
```

## Bus-Off Counting Rules

### What Counts as "Consecutive"?

**Strict interpretation**:
- No successful transmission between Bus-Off events
- Counter resets after any successful TX

**Lenient interpretation**:
- Count all Bus-Off events within time window
- Counter resets after time window expires

### Recommended Approach

```c
// Reset counter if no Bus-Off for 1 second
#define BUSOFF_RESET_TIMEOUT_MS  1000

void BusOff_Count_With_Timeout(void)
{
    static uint8_t busoff_count = 0;
    static uint32_t last_busoff = 0;
    uint32_t now = HAL_GetTick();
    
    if (CAN->ESR & CAN_ESR_BOFF) {
        // New Bus-Off event
        busoff_count++;
        last_busoff = now;
    } else {
        // Check if should reset counter
        if (now - last_busoff > BUSOFF_RESET_TIMEOUT_MS) {
            busoff_count = 0;
        }
    }
}
```

## Impact on System

### During Bus-Off

- Node cannot transmit or receive
- Other nodes may timeout waiting for messages
- Network management should detect missing node

### Recovery Considerations

1. **Network Management**:
   - Other nodes should not immediately declare node dead
   - Allow time for recovery attempts

2. **Application Layer**:
   - Buffer important messages for retransmission
   - Graceful degradation of features

3. **Safety Systems**:
   - Fail-safe states should activate
   - Driver/operator notification if applicable

## Troubleshooting Bus-Off

### Root Causes

| Cause | Symptom | Solution |
|-------|---------|----------|
| Physical layer fault | Immediate Bus-Off | Check wiring, termination |
| EMI/EMC issues | Intermittent Bus-Off | Improve shielding, filtering |
| Baud rate mismatch | Consistent errors | Verify all nodes same rate |
| Transceiver failure | No recovery possible | Replace transceiver |
| Bus overload | Errors under load | Reduce message rate |

### Diagnostic Steps

1. Check physical layer (termination, wiring)
2. Verify baud rate on all nodes
3. Monitor error counters before Bus-Off
4. Analyze error frame patterns
5. Check for EMI sources
6. Test with reduced bus load
