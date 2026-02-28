# CAN Fault Isolation

## Overview

Systematic approach to isolate CAN bus faults when multiple nodes
are involved or the problem source is unclear.

## Initial Assessment

### Step 1: Determine Scope

| Question | If Yes | If No |
|----------|--------|-------|
| All nodes affected? | Bus-wide issue | Single node issue |
| Only one node fails? | Node-specific | Continue isolation |
| Loopback works? | Internal OK | Controller problem |

### Step 2: Check Error Counters

```c
CAN_Error_t err = CAN_GetErrorStatus();

if (err.bus_off) {
    // Critical: Node excluded from bus
}
if (err.tec > err.rec) {
    // TX-side problem
}
if (err.rec > err.tec) {
    // RX/bus-side problem
}
```

## Isolation Procedure

### Phase 1: Single Node Test

```
1. Configure node in loopback mode
2. Verify TX and RX work internally
3. Check error counters stay low

If FAIL: Problem is in node itself
If PASS: Problem is external (bus/other nodes)
```

### Phase 2: Minimal Bus Test

```
1. Connect only 2 nodes
2. Verify bidirectional communication
3. Check both nodes' error counters

If FAIL: Check between these 2 nodes
If PASS: Add nodes one by one
```

### Phase 3: Node Addition

```
For each additional node:
1. Add node to bus
2. Verify communication
3. Check error counters on all nodes

When errors appear: Last added node is problematic
or network exceeds limits (nodes/cable length)
```

## Common Fault Patterns

### Pattern 1: One Node Bus-Off

**Symptoms:**
- Single node TEC > 255
- Other nodes communicate fine

**Investigation:**
1. Check node transceiver
2. Verify MCU CAN peripheral
3. Check TX timing
4. Verify firmware configuration

**Isolation:**
```c
// Test node in isolation (loopback)
CAN->BTR |= CAN_BTR_LBKM;  // Enable loopback

// If loopback works: transceiver/wiring issue
// If loopback fails: controller issue
```

### Pattern 2: All Nodes Error Passive

**Symptoms:**
- Multiple nodes with high error counters
- Intermittent communication

**Investigation:**
1. Check bus termination
2. Measure bus voltage levels
3. Check for shorts
4. Verify all baud rates match

**Isolation:**
```
1. Remove all nodes
2. Measure bus resistance (should be open)
3. Add nodes one at a time
4. Identify which node causes issues
```

### Pattern 3: Intermittent Errors

**Symptoms:**
- Random error frames
- Communication usually works

**Investigation:**
1. Check signal integrity
2. Look for EMI sources
3. Verify grounding
4. Check cable quality

**Isolation:**
```
1. Use oscilloscope on bus
2. Capture error moments
3. Correlate with external events
4. Move/shield cable to find EMI source
```

### Pattern 4: Directional Failure

**Symptoms:**
- Node A → Node B works
- Node B → Node A fails

**Investigation:**
1. Check Node B TX capability
2. Check Node A RX capability
3. Verify filters on Node A
4. Check ACK from Node A

**Isolation:**
```c
// Test Node B TX alone
// Use scope to verify signal
// Check Node A filter matches Node B's ID
```

## Hardware Isolation Tests

### Transceiver Test

```
1. Power off MCU, power on transceiver
2. Apply signal to TXD
3. Check CAN_H/CAN_L output

Expected: Differential output on bus
If missing: Transceiver failed
```

### Bus Isolation Test

```
1. Disconnect all transceivers
2. Measure between CAN_H and CAN_L
3. Expected: 60Ω (with 2 terminations)

Open circuit: Missing termination
Short circuit: Bus wiring fault
Wrong resistance: Wrong termination value
```

### MCU CAN Peripheral Test

```c
// Test without external bus
CAN->BTR |= CAN_BTR_LBKM | CAN_BTR_SILM;  // Loopback + Silent

// Send message
CAN_Transmit(&test_msg);

// Check if received in loopback
// If not: CAN peripheral issue
```

## Decision Tree

```
CAN Fault
    │
    ├── All nodes affected?
    │   │
    │   ├── Yes ──→ Check bus (termination, wiring, ground)
    │   │
    │   └── No ──→ Single node issue
    │                │
    │                ├── Loopback OK?
    │                │   │
    │                │   ├── Yes ──→ Transceiver/wiring
    │                │   │
    │                │   └── No ──→ CAN peripheral
    │                │
    │                └── Check error counters
    │                    │
    │                    ├── High TEC ──→ TX problem
    │                    │
    │                    └── High REC ──→ RX/bus problem
    │
    └── Check error types
        │
        ├── ACK errors ──→ No other active node
        │
        ├── CRC errors ──→ Signal integrity
        │
        ├── Bit errors ──→ Transceiver/bus
        │
        └── Stuff errors ──→ Baud rate mismatch
```

## Fault Documentation

When isolating faults, document:

1. **Symptoms**
   - Which nodes affected
   - Error types observed
   - Error counter values

2. **Test Results**
   - Loopback results
   - Minimal bus results
   - Hardware measurements

3. **Resolution**
   - Root cause
   - Fix applied
   - Verification method
