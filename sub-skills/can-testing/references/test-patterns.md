# CAN Test Patterns

## Overview

Standard test patterns for CAN driver verification and testing.

## Basic Test Patterns

### Pattern 1: Counter Pattern

Simple incrementing data for basic verification.

```
Message Structure:
ID: Fixed (e.g., 0x123)
Data: [counter, counter, counter, ...]

Example sequence:
Frame 1: ID=0x123, Data=[0x00, 0x00, 0x00, 0x00]
Frame 2: ID=0x123, Data=[0x01, 0x01, 0x01, 0x01]
Frame 3: ID=0x123, Data=[0x02, 0x02, 0x02, 0x02]
...
```

**Use Case:** Basic TX/RX verification, data integrity check

### Pattern 2: Ping-Pong Pattern

Request-response communication test.

```
Node A                          Node B
  │                               │
  │─── Request (ID=0x100) ────────►│
  │                               │
  │◄── Response (ID=0x200) ───────│
  │                               │
  │─── Request (ID=0x100) ────────►│
  │                               │
  │◄── Response (ID=0x200) ───────│
```

**Use Case:** Bidirectional communication, latency measurement

### Pattern 3: Broadcast Pattern

Same message received by multiple nodes.

```
               ┌────── Node B
               │
Node A ────────┼────── Node C
               │
               └────── Node D

Node A sends: ID=0x7FF, Data=[test data]
All other nodes should receive same message
```

**Use Case:** Multi-node reception, filter verification

### Pattern 4: Mixed ID Pattern

Various ID priorities to test arbitration.

```
Priority Order (lower ID = higher priority):
1. ID=0x001 (highest priority)
2. ID=0x100
3. ID=0x200
4. ID=0x7FF (lowest priority)

Test: Send all simultaneously, verify order received
```

**Use Case:** Arbitration verification, priority handling

### Pattern 5: Maximum Load Pattern

Full bus utilization test.

```
Calculate maximum frames per second:
Max frames/sec = Baud_Rate / (Bits_per_frame × 1000)

For 500 kbps, standard frame, 8 bytes:
~44,000 frames/sec theoretical
~35,000 frames/sec practical

Send at target rate and verify no loss
```

**Use Case:** Stress testing, performance validation

## Advanced Test Patterns

### Pattern 6: Boundary Values

Test edge cases in data fields.

```
Data Length Codes (DLC):
DLC=0: 0 bytes
DLC=1: 1 byte
DLC=8: 8 bytes (max for CAN 2.0)

Data Values:
0x00, 0xFF (boundary values)
0x55, 0xAA (alternating patterns)
```

### Pattern 7: Extended ID Pattern

Test 29-bit extended identifiers.

```
Standard ID: 11-bit (0x000 - 0x7FF)
Extended ID: 29-bit (0x00000000 - 0x1FFFFFFF)

Test cases:
ID=0x00000000 (min extended)
ID=0x1FFFFFFF (max extended)
ID=0x12345678 (arbitrary)
```

### Pattern 8: Remote Frame Pattern

Test remote transmission request.

```
Node A sends Remote Frame:
ID=0x100, RTR=1, DLC=8

Node B responds with Data Frame:
ID=0x100, RTR=0, DLC=8, Data=[response]
```

### Pattern 9: Error Injection Pattern

Test error handling capabilities.

```
Error scenarios to test:
1. No ACK (single node on bus)
2. Short bus (physical layer error)
3. Baud rate mismatch (stuff errors)
4. Overload (high message rate)
```

## Test Implementation

### Test Framework Structure

```c
typedef struct {
    uint32_t id;
    uint8_t ide;     // 0=standard, 1=extended
    uint8_t rtr;     // 0=data, 1=remote
    uint8_t dlc;
    uint8_t data[8];
} CAN_TestFrame_t;

typedef struct {
    uint32_t tx_count;
    uint32_t rx_count;
    uint32_t error_count;
    uint32_t lost_count;
} CAN_TestStats_t;
```

### Test Sequence

```c
void CAN_RunTestSequence(void)
{
    // Test 1: Basic TX/RX
    CAN_Test_BasicLoopback();
    
    // Test 2: Counter pattern
    CAN_Test_CounterPattern(100);  // 100 iterations
    
    // Test 3: Mixed IDs
    CAN_Test_MixedID();
    
    // Test 4: Stress test
    CAN_Test_Stress(10000);  // 10000 frames
    
    // Test 5: Filter test
    CAN_Test_Filter();
    
    // Print results
    CAN_Test_PrintResults();
}
```

## Performance Metrics

| Metric | Target | Measurement |
|--------|--------|-------------|
| TX success rate | 100% | TX_count / TX_attempts |
| RX success rate | 100% | RX_count / TX_count |
| Latency | < 1ms | Time TX to RX |
| Throughput | > 95% theoretical | Actual frames/sec |
| Error rate | 0% | Errors / Total frames |

## Test Report Template

```
=== CAN Test Report ===

Configuration:
  Baud Rate: XXX kbps
  Sample Point: XX.X%
  Filter: [settings]

Results:
  TX Frames: XXXX
  RX Frames: XXXX
  Errors: XX
  Lost: XX

Pass/Fail: [PASS/FAIL]

Notes:
  [Any observations]
```
