---
name: can-debug
description: |
  CAN bus debugging and analysis. Analyzes signal quality, timing issues,
  and hardware problems. Use when communication is unstable or has errors.
  Requires oscilloscope/logic analyzer data in most cases.
version: 1.0.0
---

# CAN Debug Analysis

Debug and analyze CAN bus communication issues.

## Trigger Conditions

- CAN communication unstable
- Missing messages
- Intermittent errors
- Baud rate mismatch suspected
- Signal quality issues

## Workflow

### Step 1: Gather Debug Information

Ask user for available information:
- Oscilloscope/logic analyzer captures
- Error statistics from CAN controller
- Description of symptoms
- Network topology (number of nodes, cable length)

### Step 2: Hardware Check

Read `references/hardware-check.md` for checklist:

- [ ] CAN_H and CAN_L not swapped
- [ ] 120-ohm termination at both ends
- [ ] Proper transceiver power supply
- [ ] No short circuits on bus lines
- [ ] Correct pin connections

### Step 3: Signal Quality Analysis

Read `references/signal-analysis.md` for analysis guide.

Key signal parameters:
| Parameter | Expected | Problem |
|-----------|----------|---------|
| Recessive level | ~2.5V (diff: 0V) | Bias issues |
| Dominant level | ~1.5V/3.5V (diff: 2V) | Weak driver |
| Edge slope | 3-50 ns/V | Slow edges, ringing |
| Sample point | ~87.5% | Timing mismatch |

Run `scripts/can_bit_timing.py --analyze <capture>` if waveform data available.

### Step 4: Timing Debug

Read `references/timing-debug.md` for timing issues.

Common timing problems:
- Baud rate mismatch between nodes
- Sample point position mismatch
- Propagation delay too long
- Clock drift

### Step 5: Error Analysis

Query CAN controller error counters:
- TEC (Transmit Error Counter)
- REC (Receive Error Counter)
- Error status flags

| Counter | State |
|---------|-------|
| 0-7 | Error active |
| 8-127 | Warning |
| 128-255 | Error passive |
| 255+ | Bus off |

### Step 6: Generate Debug Report

Summarize findings:
1. Identified issues
2. Root cause analysis
3. Recommended fixes
4. Verification steps

## Debug Checklist

### Physical Layer
- [ ] Transceiver power (5V typical)
- [ ] Bus termination (120 ohm)
- [ ] Wire polarity (CAN_H/CAN_L)
- [ ] Cable quality and length
- [ ] Connector integrity

### Data Link Layer
- [ ] Baud rate configuration
- [ ] Sample point position
- [ ] Filter configuration
- [ ] Interrupt priorities

### Protocol
- [ ] ID conflicts
- [ ] Message timing
- [ ] ACK slot issues

## Common Issues and Solutions

| Symptom | Likely Cause | Debug Step |
|---------|--------------|------------|
| No communication | Hardware | Check termination, power |
| Random errors | Signal quality | Check oscilloscope capture |
| Only TX works | No other node | Verify other nodes powered |
| Baud rate drift | Clock issue | Check crystal/PLL |
| Error passive | Bus overload | Reduce message rate |

## Tools

- `scripts/can_bit_timing.py` - Calculate/verify bit timing parameters

## Reference Files

- `references/hardware-check.md` - Hardware debugging checklist
- `references/signal-analysis.md` - Signal quality analysis guide
- `references/timing-debug.md` - Timing troubleshooting
