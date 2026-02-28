---
name: can-testing
description: |
  CAN driver testing and verification. Generates loopback tests, stress tests,
  and verification code. Use when validating CAN driver functionality or
  performing integration testing.
version: 1.0.0
---

# CAN Testing

Generate test code for CAN driver verification.

## Trigger Conditions

- Verify CAN driver functionality
- Loopback testing
- Stress/benchmark testing
- Integration testing
- Regression testing

## Workflow

### Step 1: Determine Test Type

| Test Type | Purpose | Requires |
|-----------|---------|----------|
| Loopback | Basic TX/RX | Single node |
| External | Real communication | Two+ nodes |
| Stress | Performance/robustness | Optional load |
| Error | Error handling | Error injection |

### Step 2: Loopback Test

Read `references/test-patterns.md` for test patterns.

Generate loopback test code:

```
Read assets/loopback-test.template.c
```

Loopback test verifies:
- TX mailbox operation
- RX FIFO reception
- ID filtering
- Data integrity
- Interrupt handling

### Step 3: External Communication Test

For two-node communication test:
1. Connect two CAN nodes
2. Run TX test on node A
3. Verify RX on node B
4. Reverse direction

### Step 4: Stress Test

Generate stress test code:

```
Read assets/stress-test.template.c
```

Stress test parameters:
- Message rate (messages/second)
- Payload size (0-8 bytes for CAN, 0-64 for CAN-FD)
- Duration (seconds)
- Target fill level (bus load %)

### Step 5: Error Injection Test

Test error handling by:
- Disabling one node (ACK error)
- Shorting bus lines (bit error)
- Overloading bus (error passive)

## Test Patterns

Read `references/test-patterns.md` for standard test patterns:

| Pattern | Description |
|---------|-------------|
| Counter | Incrementing data payload |
| Ping-Pong | Request-response sequence |
| Broadcast | Same ID, multiple nodes |
| Mixed-ID | Various ID priorities |
| Maximum Load | Full bus utilization |

## Verification Checklist

### Basic Functionality
- [ ] CAN init succeeds
- [ ] TX mailbox available
- [ ] TX complete interrupt fires
- [ ] RX message received
- [ ] Data matches TX
- [ ] ID matches TX

### Filter Testing
- [ ] Exact ID match works
- [ ] Mask filtering works
- [ ] Rejected messages filtered

### Error Handling
- [ ] Bus-off recovery works
- [ ] Error counters readable
- [ ] Error interrupts fire
- [ ] Recovery after error

### Performance
- [ ] Max message rate achieved
- [ ] No message loss at target rate
- [ ] Interrupt latency acceptable

## Test Output Analysis

### Success Indicators
- All TX messages sent
- All RX messages received
- No unexpected errors
- Error counters stay low

### Failure Indicators
- TX mailbox timeout
- RX message loss
- Unexpected error frames
- Bus-off state

## Template Files

- `assets/loopback-test.template.c` - Loopback test code
- `assets/stress-test.template.c` - Stress test code

## Reference Files

- `references/test-patterns.md` - Standard test patterns
