---
name: can-skill
description: |
  Embedded MCU CAN/CAN-FD driver development skill. Supports driver development,
  debugging, diagnosis, and testing for CAN bus. Works with STM32, NXP, Infineon
  and other mainstream MCUs. Do NOT use for Linux SocketCAN or PC-based CAN tools.
version: 1.0.0
author: cyj0920
tags: [can, can-fd, embedded, mcu, driver, automotive, bus]
---

# CAN Skill

A comprehensive skill for embedded MCU CAN bus development, covering driver
implementation, debugging, diagnosis, and testing phases.

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
└── assets/                           # Code templates
```

## Notes

- All generated code uses pure C with register-level operations
- CAN-FD support is optional, loaded on demand from `references/can-fd-extension.md`
- For specific MCU adaptations, user should verify register addresses
