---
name: can-skill
description: |
  Embedded MCU CAN/CAN-FD driver development skill. Supports driver development,
  debugging, diagnosis, and testing for CAN bus. Works with STM32, NXP, Infineon
  and other mainstream MCUs. Do NOT use for Linux SocketCAN or PC-based CAN tools.
version: 1.1.0
author: cyj0920
tags: [can, can-fd, embedded, mcu, driver, automotive, bus]
---

# CAN Skill

A comprehensive skill for embedded MCU CAN bus development, covering driver
implementation, debugging, diagnosis, and testing phases.

## Quick Diagnosis

**Over 80% of CAN issues are physical layer problems.** Start here:

1. **Check termination**: Measure CAN_H to CAN_L resistance (should be ~60Ω)
2. **Check idle voltage**: CAN_H and CAN_L should be ~2.5V each
3. **Check baud rate**: All nodes must match exactly (< 0.5% tolerance)
4. **Check wiring**: CAN_H to CAN_H, CAN_L to CAN_L (not swapped)

For detailed cases, see `references/common-fault-cases.md`.

## Trigger Conditions

Invoke this skill when the user needs to:
- Develop CAN/CAN-FD driver for MCU
- Debug CAN communication issues
- Diagnose CAN bus faults
- Generate CAN test code

Do NOT trigger for:
- Linux SocketCAN development
- PC-based CAN analysis tools
- CANopen/DeviceNet higher-layer protocols

## Workflow

### Step 1: Identify Task Type

Determine the task category based on user request:

| Task Keywords | Route To |
|---------------|----------|
| init, configure, transmit, receive, interrupt, filter | `sub-skills/can-driver-dev/SKILL.md` |
| debug, analyze, waveform, timing, signal | `sub-skills/can-debug/SKILL.md` |
| error, fault, diagnose, not working, fail | `sub-skills/can-diagnosis/SKILL.md` |
| test, verify, loopback, stress | `sub-skills/can-testing/SKILL.md` |

### Step 2: Gather MCU Context

Ask user for target MCU platform if not specified:
- STM32 (F1/F4/H7 series)
- NXP (S32K/LPC series)
- Infineon (Aurix/TC series)
- Other (generic register-level code)

### Step 3: Load Protocol Basics

Read `references/can-protocol.md` for CAN protocol fundamentals.

### Step 4: Execute Sub-Skill

Follow the specific sub-skill workflow based on task type:

1. **Driver Development**: See `sub-skills/can-driver-dev/SKILL.md`
2. **Debug Analysis**: See `sub-skills/can-debug/SKILL.md`
3. **Problem Diagnosis**: See `sub-skills/can-diagnosis/SKILL.md`
4. **Testing**: See `sub-skills/can-testing/SKILL.md`

## Reference Documents

### Protocol & Standards
- `references/can-protocol.md` - CAN protocol fundamentals
- `references/can-fd-extension.md` - CAN-FD extensions and migration
- `references/common-mcu-can.md` - MCU CAN controller overview

### Troubleshooting
- `references/common-fault-cases.md` - 10 real-world fault cases
- `references/emi-emc-guide.md` - EMI/EMC design guide

## Output Format

### For Code Generation
- Pure C code with register-level operations
- Include detailed comments for key configurations
- Provide compile-time configurable macros

### For Q&A
- Provide concise technical explanations
- Reference specific register names when applicable
- Include code snippets for illustration

### For Diagnosis
- Provide step-by-step troubleshooting checklist
- Explain root cause analysis
- Suggest verification methods

## File Structure

```
can-skill/
├── SKILL.md                          # This file
├── sub-skills/
│   ├── can-driver-dev/               # Driver development
│   ├── can-debug/                    # Debugging analysis
│   ├── can-diagnosis/                # Problem diagnosis
│   └── can-testing/                  # Testing verification
├── references/                       # Protocol documentation
│   ├── can-protocol.md
│   ├── can-fd-extension.md
│   ├── common-mcu-can.md
│   ├── common-fault-cases.md
│   └── emi-emc-guide.md
└── assets/                           # Code templates
```

## Common Error Quick Reference

| Error Type | LEC | Likely Cause | First Check |
|------------|-----|--------------|-------------|
| ACK Error | 3 | No other nodes active | Verify other nodes powered |
| CRC Error | 6 | Signal integrity | Check termination, cable |
| Bit Error | 4-5 | Transceiver/wiring | Check bus voltages |
| Stuff Error | 1 | Baud rate mismatch | Verify all nodes same rate |
| Form Error | 2 | Timing/noise | Check sample point |

## Notes

- All generated code uses pure C with register-level operations
- CAN-FD support is optional, loaded on demand from `references/can-fd-extension.md`
- For specific MCU adaptations, user should verify register addresses
- Physical layer checks should always precede software debugging