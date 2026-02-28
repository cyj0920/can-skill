---
name: can-driver-dev
description: |
  CAN/CAN-FD driver development for embedded MCUs. Generates initialization,
  transmission, reception, interrupt handling, and filter configuration code.
  Use when developing CAN driver from scratch or porting to new MCU.
version: 1.0.0
---

# CAN Driver Development

Generate register-level CAN driver code for embedded MCUs.

## Prerequisites

Confirm with user:
1. Target MCU family (STM32/NXP/Infineon/Other)
2. CAN or CAN-FD mode
3. Required baud rate
4. RX interrupt or polling mode

## Workflow

### Step 1: Clock Configuration

Configure CAN peripheral clock:
- Enable CAN module clock in RCC/CCM
- Calculate optimal prescaler value
- Verify APB clock frequency

Read `references/timing-config.md` for baud rate calculation.

### Step 2: GPIO Configuration

Configure CAN TX/RX pins:
- Set alternate function mode
- Configure pull-up/pull-down
- Set pin speed (high speed recommended)

### Step 3: CAN Initialization

Generate initialization code based on template:

```
Read assets/can-init.template.c
```

Key configuration steps:
1. Enter initialization mode (INRQ=1, wait for INAK)
2. Configure timing (BTR register)
3. Configure filters
4. Enter normal mode (INRQ=0, wait for INAK clear)

Read `references/register-bits.md` for register details.

### Step 4: Filter Configuration

Configure acceptance filters:

```
Read assets/can-filter.template.c
```

Read `references/filter-config.md` for filter modes:
- Mask mode vs List mode
- Identifier filtering
- 32-bit vs 16-bit filter width

### Step 5: Transmit Implementation

Generate TX code:

```
Read assets/can-tx.template.c
```

Key points:
- Select empty TX mailbox
- Configure ID, DLC, data
- Request transmission
- Handle TX complete

### Step 6: Receive Implementation

Generate RX code based on mode:

**Interrupt Mode** (recommended):
```
Read assets/can-rx.template.c
```
Read `references/interrupt-setup.md` for ISR configuration.

**Polling Mode**:
Check RX FIFO not empty, then read data.

### Step 7: Interrupt Configuration (if required)

Configure NVIC for CAN interrupts:
- RX FIFO message pending
- TX mailbox complete
- Error interrupts

Read `references/interrupt-setup.md` for details.

## Output Checklist

Generated code should include:
- [ ] Clock enable function
- [ ] GPIO configuration
- [ ] CAN init function
- [ ] Filter configuration
- [ ] Transmit function
- [ ] Receive function/ISR
- [ ] Error handling

## Common Issues

| Issue | Cause | Solution |
|-------|-------|----------|
| Cannot enter init mode | Clock not enabled | Check RCC configuration |
| No RX messages | Filter too strict | Verify filter mask/ID |
| Baud rate mismatch | Timing wrong | Use scripts/can_bit_timing.py |
| TX stuck | No empty mailbox | Add mailbox wait timeout |

## Reference Files

- `references/timing-config.md` - Baud rate calculation
- `references/register-bits.md` - Key register definitions
- `references/filter-config.md` - Filter configuration guide
- `references/interrupt-setup.md` - Interrupt handling

## Template Files

- `assets/can-init.template.c` - Initialization code
- `assets/can-tx.template.c` - Transmit code
- `assets/can-rx.template.c` - Receive code
- `assets/can-filter.template.c` - Filter configuration
