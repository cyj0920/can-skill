# CAN Buffer Modes: Basic CAN vs Full CAN

## Overview

CAN controllers offer different modes for managing message buffers.
Understanding these modes is essential for proper configuration and
avoiding common pitfalls.

## Basic Concepts

### Hardware Object (HOH)

Hardware Objects are RAM regions that store CAN message information:
- **HRH**: Hardware Receive Handle
- **HTH**: Hardware Transmit Handle

Each HOH can be configured for:
- Dedicated buffer (single message)
- FIFO buffer (multiple messages)
- Queue buffer (prioritized)

## Full CAN Mode

### Definition

**One HOH = One CAN message**

Each hardware object is dedicated to a single CAN ID.

### Characteristics

| Property | Value |
|----------|-------|
| Buffer type | Dedicated |
| Messages per HOH | 1 |
| ID filtering | Hardware (exact match) |
| Overrun handling | Not applicable |
| RAM usage | Higher per message |

### Advantages

- Deterministic latency
- Simple software handling
- Direct message access
- No overflow possible per buffer

### Disadvantages

- Higher RAM consumption
- Limited number of messages
- Less flexible filtering

### Configuration Example

```
Message RAM Layout (Full CAN):
┌─────────────┐
│ HRH ID=0x100│ ← Dedicated to ID 0x100
├─────────────┤
│ HRH ID=0x200│ ← Dedicated to ID 0x200
├─────────────┤
│ HRH ID=0x300│ ← Dedicated to ID 0x300
├─────────────┤
│ HTH         │ ← Transmit buffer
└─────────────┘
```

## Basic CAN Mode

### Definition

**One HOH = Multiple CAN messages**

A single hardware object can receive or transmit multiple CAN IDs.

### Characteristics

| Property | Value |
|----------|-------|
| Buffer type | FIFO or Dedicated |
| Messages per HOH | Multiple (configurable depth) |
| ID filtering | Software or hardware mask |
| Overrun handling | Watermark interrupt |
| RAM usage | More efficient |

### Advantages

- Lower RAM consumption
- Flexible ID filtering
- Can handle many messages
- Suitable for diagnostic queues

### Disadvantages

- Potential overflow
- Non-deterministic latency
- Requires careful depth configuration

### Configuration Example

```
Message RAM Layout (Basic CAN):
┌─────────────────────────────┐
│         FIFO Buffer         │
│    Depth = 4 elements       │
│  ┌─────┬─────┬─────┬─────┐  │
│  │0x100│0x200│0x150│0x180│  │ ← Multiple IDs in one FIFO
│  └─────┴─────┴─────┴─────┘  │
│         HRH (Basic)         │
└─────────────────────────────┘
```

## FIFO Mode

### What is FIFO?

FIFO (First In, First Out) is a buffer structure where:
- Messages are stored in arrival order
- Oldest message is read first
- Depth determines buffer capacity

### FIFO Operation

```
Incoming: ID 0x80 → 0x70 → 0x100 → 0x50
                    ↓
              ┌─────────────────────┐
              │  0x80 │ 0x70 │ 0x100│ 0x50 │
              └─────────────────────┘
                    ↓
Read order: 0x80 first, then 0x70, etc.
```

### Ring Buffer Implementation

Modern CAN controllers use ring buffers:

```
Put Index (write) → ┌─────┬─────┬─────┬─────┐
                   │ Msg │ Msg │ Msg │ Msg │
                   └─────┴─────┴─────┴─────┘
                                ↑ Get Index (read)

When Put Index catches Get Index: Buffer Full
```

### Watermark Interrupt

To prevent overflow, use watermark:

```c
// Configure watermark at 75% of depth
// For depth=4, watermark at 3
RXFIFO->WATERMARK = 3;

// When 3+ messages pending, trigger interrupt
// ISR reads messages before overflow
```

## Comparison Table

| Feature | Full CAN | Basic CAN |
|---------|----------|-----------|
| Messages per HOH | 1 | Multiple |
| RAM efficiency | Lower | Higher |
| Latency | Deterministic | Variable |
| Overflow risk | None | Possible |
| ID filtering | Exact match | Mask-based |
| Use case | Real-time control | Diagnostics, bulk data |
| Software complexity | Simple | Moderate |

## Common Pitfall: FIFO Depth Configuration

### Problem Scenario

**Symptom**: 200ms periodic message received every 6.4 seconds

**Root Cause**: 
- Message configured as Basic CAN with FIFO depth = 32
- Interrupt only triggers when FIFO reaches watermark
- 32 messages × 200ms = 6.4 seconds

### Incorrect Configuration

```
Message: ID 0x100, Period 200ms
Config:  Basic CAN, FIFO depth=32, watermark=32

Result: Interrupt fires every 6.4 seconds!
```

### Correct Configuration

```
Option 1: Use Full CAN (Dedicated buffer)
Message: ID 0x100, Period 200ms
Config:  Full CAN, dedicated HRH
Result:  Interrupt fires every 200ms ✓

Option 2: Use Basic CAN with depth=1
Message: ID 0x100, Period 200ms  
Config:  Basic CAN, FIFO depth=1
Result:  Interrupt fires every 200ms ✓
```

### Code Fix

```c
// Wrong: Deep FIFO for single periodic message
// Config->HRH[0].Depth = 32;  // Causes 6.4s delay!

// Correct: Single element FIFO
Config->HRH[0].Mode = BASIC_CAN;
Config->HRH[0].Depth = 1;  // Immediate notification
Config->HRH[0].Watermark = 1;
```

## When to Use Each Mode

### Use Full CAN When:

- Real-time control messages
- Deterministic latency required
- Low message count
- Safety-critical signals

### Use Basic CAN When:

- Diagnostic message queues
- Bootloader data transfer
- High message count
- Memory constraints
- Bulk data reception

### Use FIFO When:

- Message order must be preserved
- Queue-style processing needed
- Multiple similar messages expected

## Message RAM Allocation

### STM32 Example

```
CAN Message RAM (typically 2560 words):
├── Filter Configuration (128 words max)
├── Rx FIFO 0 (configurable)
├── Rx FIFO 1 (configurable)  
├── Rx Buffer / Dedicated HRHs
├── Tx Event FIFO
├── Tx Buffers / Dedicated HTHs
└── Tx Queue

Total must not exceed available RAM!
```

### Allocation Strategy

1. Calculate total HRH + HTH needed
2. Account for FIFO depths
3. Reserve margin for future expansion
4. Verify against hardware limits

## Example Configurations

### Low-Resource System

```
Full CAN Mode:
- 16 HRH (16 receive messages)
- 4 HTH (4 transmit messages)
- No FIFOs
Total: 20 hardware objects
```

### Diagnostic-Heavy System

```
Mixed Mode:
- 4 Full CAN HRH (critical signals)
- 1 Basic CAN FIFO (diagnostics, depth=64)
- 2 HTH (normal + diagnostic TX)
```

### Bootloader Application

```
Basic CAN Only:
- 1 Rx FIFO (depth=64, receive firmware chunks)
- 1 Tx FIFO (depth=8, send responses)
```

## Best Practices

1. **Match depth to message rate**: 
   - High rate → deeper FIFO
   - Low rate → shallow FIFO or Full CAN

2. **Set appropriate watermarks**:
   - Alert before overflow
   - Leave time for ISR processing

3. **Monitor overflow flags**:
   - Log overflow events
   - Adjust depth if needed

4. **Consider ISR latency**:
   - Deep FIFO allows more ISR latency
   - But increases message latency

5. **Document configuration**:
   - Why each depth was chosen
   - Impact of changes
