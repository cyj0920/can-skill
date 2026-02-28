---
name: can-diagnosis
description: |
  CAN bus fault diagnosis and troubleshooting. Diagnoses error frames,
  bus states, and communication failures. Use when CAN is not working
  or showing persistent errors.
version: 1.0.0
---

# CAN Problem Diagnosis

Diagnose and troubleshoot CAN bus communication problems.

## Trigger Conditions

- CAN not working at all
- Persistent error frames
- Bus-off state
- Communication failures
- Error counter overflow

## Workflow

### Step 1: Identify Current Bus State

Read `references/bus-states.md` for state definitions.

Query controller status registers:

| State | TEC | REC | Behavior |
|-------|-----|-----|----------|
| Error Active | 0-127 | 0-127 | Normal operation |
| Error Passive | 128-255 | 128-255 | Limited TX/RX |
| Bus Off | >255 | - | No TX/RX |

### Step 2: Analyze Error Frames

Read `references/error-frames.md` for error types.

Check error flags:

| Error Type | Cause | Bits Affected |
|------------|-------|---------------|
| Bit Error | TX/RX mismatch | Single bit |
| Stuff Error | 6 same bits | Bit stuffing |
| CRC Error | CRC mismatch | CRC field |
| Form Error | Invalid format | EOF, IFS |
| ACK Error | No acknowledgment | ACK slot |

### Step 3: Isolate Fault

Read `references/fault-isolation.md` for isolation procedure.

Fault isolation steps:
1. Check if single node or all nodes affected
2. Remove nodes one by one
3. Check bus voltage levels
4. Analyze error pattern

### Step 4: Root Cause Analysis

Based on symptoms, identify root cause:

| Symptom | Likely Cause |
|---------|--------------|
| Bus off on single node | Node hardware failure |
| All nodes bus off | Bus short, no termination |
| ACK errors | No other active node |
| CRC errors | Signal integrity, timing |
| Stuff errors | Baud rate mismatch |
| Bit errors | Driver issue, bus contention |

### Step 5: Generate Diagnosis Report

Create structured report using template:

```
Read assets/diagnosis-report.template.md
```

Report includes:
- Current bus state
- Error analysis
- Identified root cause
- Recommended fix
- Verification procedure

## Quick Diagnosis Flowchart

```
CAN Not Working
      |
      v
Check Bus Off? ----Yes---> Check TEC, power cycle
      |
      No
      v
Check Error Counters ----High---> Analyze error type
      |
      Normal
      v
Check Filter Config ----Wrong---> Fix filter
      |
      Correct
      v
Check Hardware --------Problem---> Fix wiring/termination
```

## Common Fault Scenarios

### Scenario 1: No Communication at All

1. Check transceiver power
2. Check termination resistors
3. Verify CAN_H/CAN_L polarity
4. Check bus voltage (idle ~2.5V differential)

### Scenario 2: Random Error Frames

1. Check signal integrity (oscilloscope)
2. Verify all nodes same baud rate
3. Check ground connections
4. Look for EMI sources

### Scenario 3: Single Node Failing

1. Check node transceiver
2. Verify MCU CAN peripheral
3. Check interrupt configuration
4. Test with different firmware

### Scenario 4: Bus Off Recovery

1. Identify cause of bus off
2. Fix root cause
3. Implement bus-off recovery in code:
```c
// Typical recovery sequence
CAN->MCR |= CAN_MCR_ABOM;  // Enable auto bus-off recovery
// Or manual recovery:
CAN->MCR |= CAN_MCR_INRQ;  // Request init mode
while(!(CAN->MSR & CAN_MSR_INAK));  // Wait acknowledge
CAN->MCR &= ~CAN_MCR_INRQ;  // Exit init mode
```

## Reference Files

- `references/bus-states.md` - Bus state definitions
- `references/error-frames.md` - Error frame analysis
- `references/fault-isolation.md` - Fault isolation procedure

## Template Files

- `assets/diagnosis-report.template.md` - Diagnosis report template
