# Common CAN Fault Cases

This document presents real-world CAN bus fault cases with analysis and solutions.

## Case 1: Missing Termination

### Symptoms
- Random CRC errors
- Intermittent communication failures
- Signal ringing visible on oscilloscope

### Investigation
```
Measurement: CAN_H to CAN_L resistance
Result: 120Ω (expected: ~60Ω)
Conclusion: Only one termination resistor present
```

### Root Cause
Only one 120Ω terminator at the master node. Missing terminator at the far end caused signal reflections.

### Solution
Added 120Ω termination resistor at the far end node.

### Waveform Comparison
```
Before (with reflection):
    ┌───┐   ┌─────┐
    │   │   │     │
────┘   └───┘     └────
        ↑
      Ringing

After (proper termination):
    ┌───┐       ┌───┐
    │   │       │   │
────┘   └───────┘   └────
    Clean edges, no ringing
```

---

## Case 2: Baud Rate Mismatch

### Symptoms
- Stuff errors on all messages
- Nodes cannot communicate
- Bus-off condition on startup

### Investigation
```
Node A: Configured for 500 kbps
Node B: Configured for 250 kbps
Oscilloscope: Bit time mismatch visible
```

### Root Cause
Software configuration error - baud rates not matching between nodes.

### Solution
Updated Node B configuration to 500 kbps:
```c
// Node B - corrected
CAN->BTR = (9 - 1) | ((6 - 1) << 16) | ((1 - 1) << 20);
// Now matches Node A: 500 kbps @ 36 MHz
```

### Verification
After fix, both nodes showed identical bit timing on oscilloscope.

---

## Case 3: CAN_H and CAN_L Swapped

### Symptoms
- No communication
- Error counters increase rapidly
- Immediate bus-off

### Investigation
```
Voltage measurement:
  CAN_H to GND: ~1.5V (expected: ~2.5V idle)
  CAN_L to GND: ~3.5V (expected: ~2.5V idle)

Conclusion: Lines are swapped
```

### Root Cause
Incorrect wiring at DB9 connector - pins 2 and 7 swapped.

### Solution
Corrected connector wiring:
```
Before:           After:
Pin 2 → CAN_H     Pin 2 → CAN_L (correct)
Pin 7 → CAN_L     Pin 7 → CAN_H (correct)
```

---

## Case 4: EMI from Motor Drive

### Symptoms
- Errors occur only when motor is running
- CRC and bit errors reported
- Communication stable when motor off

### Investigation
```
Oscilloscope capture during motor operation:
- Large noise spikes on CAN signals
- Common mode noise ~2V peak
- Correlated with motor PWM frequency
```

### Root Cause
Motor drive cables routed parallel to CAN bus, inducing noise through magnetic coupling.

### Solutions Applied
1. Separated motor cables from CAN bus by 20cm
2. Added common mode choke at each node
3. Installed shielded CAN cable

### Result
Errors reduced from 100/hour to < 1/hour.

---

## Case 5: Ground Potential Difference

### Symptoms
- Intermittent errors
- Worse during heavy load equipment operation
- Signal levels vary between nodes

### Investigation
```
Voltage between node grounds:
  Node A GND to Node B GND: 2.5V difference
  During equipment operation: Up to 5V difference

CAN signal at Node B:
  CAN_H to local GND: 0V to 5V (outside spec)
```

### Root Cause
Nodes powered from different power supplies with no common ground reference.

### Solution
1. Connected all node grounds together
2. Added common ground wire in CAN cable
3. For isolation: used isolated CAN transceivers

---

## Case 6: Transceiver Failure

### Symptoms
- One node cannot transmit
- Can receive messages normally
- No hardware visible damage

### Investigation
```
Transmitter test:
  MCU TXD pin toggling: OK
  Transceiver CAN output: No change

Receiver test:
  CAN input changes: OK
  Transceiver RXD output: OK

Conclusion: TX circuitry failed
```

### Root Cause
ESD damage to transmitter section of CAN transceiver IC.

### Solution
Replaced transceiver IC. Added TVS protection:
```c
// Added ESD protection circuit
CAN_H ─┬─ TVS_diode ─┬─ GND
CAN_L ─┴─ TVS_diode ─┘
```

---

## Case 7: Sample Point Too Early

### Symptoms
- Errors at maximum cable length
- Works fine at short distances
- Stuff errors predominant

### Investigation
```
Configuration:
  Total Tq: 8
  TS1: 4 (Prop + PS1 = 5)
  TS2: 2 (PS2 = 2)
  Sample point: (1+4)/8 = 62.5%

Cable length: 35m @ 1 Mbps
Propagation delay: ~400ns
```

### Root Cause
Sample point too early (62.5%) - signal not settled at sampling instant due to propagation delay.

### Solution
Adjusted sample point to 87.5%:
```c
// Before: 62.5% sample point
CAN->BTR = (4 - 1) | ((4 - 1) << 16) | ((2 - 1) << 20);

// After: 87.5% sample point
CAN->BTR = (4 - 1) | ((6 - 1) << 16) | ((1 - 1) << 20);
```

---

## Case 8: Bus Overload

### Symptoms
- Overload frames occurring
- Messages delayed
- High bus utilization reported

### Investigation
```
Message analysis:
  Messages/second: 8000
  Average message: 130 bits (with stuffing)
  Bus load: 8000 × 130 / 500000 = 208%

Conclusion: Severe bus overload
```

### Root Cause
Too many messages for the configured baud rate.

### Solutions
1. Increased baud rate from 500 kbps to 1 Mbps
2. Reduced message frequency for non-critical data
3. Combined multiple signals into single messages

### Result
Bus load reduced from 208% to 45%.

---

## Case 9: Crystal Oscillator Drift

### Symptoms
- Errors occur after warm-up
- System works fine at cold start
- Errors increase over time

### Investigation
```
Clock measurement:
  Cold (25°C): 8.000 MHz
  Hot (85°C): 7.992 MHz (1000 ppm error)

Calculated baud rate error: 0.1%
Actual: 0.1% × 10 nodes = potential 1% mismatch
```

### Root Cause
Low-quality crystal with poor temperature stability.

### Solution
Replaced crystal with higher stability unit:
- Old: ±100 ppm over temperature
- New: ±20 ppm over temperature

---

## Case 10: Stubs Too Long

### Symptoms
- Signal reflections visible
- Random errors
- Multiple nodes affected

### Investigation
```
Topology analysis:
  Main bus: 30m
  Stub lengths: 2m, 3m, 5m, 2m
  
Oscilloscope:
  Reflections at stub junction points
  Ringing amplitude: ~0.5V
```

### Root Cause
Stub lengths exceeding recommended maximum (0.3m at 1 Mbps).

### Solution
Reconfigured to proper bus topology:
- Reduced stubs to < 0.3m
- Added repeaters for distant nodes
- Improved connector placement

---

## Quick Diagnosis Flowchart

```
Start
  │
  ├─ No communication at all?
  │     │
  │     ├─ Yes → Check termination (60Ω?)
  │     │         Check wiring (H/L swapped?)
  │     │         Check transceiver power
  │     │
  │     └─ No → Continue
  │
  ├─ Errors at specific times?
  │     │
  │     ├─ Yes → Look for EMI sources
  │     │         Check ground differences
  │     │
  │     └─ No → Continue
  │
  ├─ Works at short distance, fails at long?
  │     │
  │     ├─ Yes → Check sample point
  │     │         Check propagation delay
  │     │         Check termination
  │     │
  │     └─ No → Continue
  │
  ├─ Errors at specific baud rate?
  │     │
  │     ├─ Yes → Check baud rate match
  │     │         Check clock accuracy
  │     │         Check cable length
  │     │
  │     └─ No → Continue
  │
  └─ Random intermittent errors?
        │
        ├─ Yes → Check connectors
        │         Check for EMI
        │         Check ground stability
        │
        └─ No → Review specific symptoms
```

## Summary Table

| Case | Root Cause | Key Symptom | Solution |
|------|------------|-------------|----------|
| 1 | Missing termination | Ringing, CRC errors | Add 120Ω terminator |
| 2 | Baud rate mismatch | Stuff errors, bus-off | Fix configuration |
| 3 | H/L swapped | No comm, bus-off | Correct wiring |
| 4 | EMI | Motor-related errors | Separate cables, CMC |
| 5 | Ground difference | Intermittent, varying levels | Common ground |
| 6 | Transceiver failure | TX fails, RX OK | Replace, add TVS |
| 7 | Sample point early | Long distance errors | Adjust timing |
| 8 | Bus overload | Overload frames | Reduce traffic |
| 9 | Clock drift | Temperature-related | Better crystal |
| 10 | Long stubs | Reflections | Shorten stubs |
