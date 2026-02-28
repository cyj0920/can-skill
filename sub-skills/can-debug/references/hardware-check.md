# CAN Hardware Check

## Physical Layer Checklist

### 1. Power Supply

| Check | Expected | Problem Indicator |
|-------|----------|-------------------|
| Transceiver VCC | 5V (typical) | < 4.5V or > 5.5V |
| MCU VCC | Per datasheet | Voltage droop |
| Ground connection | 0V potential | Ground loop |

### 2. CAN Bus Lines

| Check | Expected | Problem Indicator |
|-------|----------|-------------------|
| CAN_H idle | ~2.5V | Not ~2.5V |
| CAN_L idle | ~2.5V | Not ~2.5V |
| Differential idle | ~0V | Non-zero |
| CAN_H dominant | ~3.5V | Wrong level |
| CAN_L dominant | ~1.5V | Wrong level |
| Differential dominant | ~2V | Wrong level |

### 3. Termination

| Check | Expected | Problem Indicator |
|-------|----------|-------------------|
| End termination | 120Ω each | Missing/wrong value |
| Total resistance | ~60Ω | Open or short |
| Single point | 120Ω | Only one terminator |

**Measurement:**
```
Power off, disconnect transceivers
Measure between CAN_H and CAN_L:
- Expected: ~60Ω (two 120Ω in parallel)
- Open: > 120Ω (missing termination)
- Short: < 50Ω (bus shorted)
```

### 4. Transceiver

| Check | Method |
|-------|--------|
| TXD input | Check MCU pin output |
| RXD output | Check MCU pin input |
| STB/EN pin | Verify standby mode disabled |
| SPLIT pin | Common mode stabilization |

### 5. Pin Connections

| Signal | Common Mistakes |
|--------|-----------------|
| CAN_H/CAN_L | Swapped |
| TXD/RXD | Swapped |
| Transceiver power | Not connected |
| Ground | Not shared |

## Debugging Tools

### Multimeter Checks

```
1. Power off:
   - Measure CAN_H to CAN_L: ~60Ω
   
2. Power on, idle bus:
   - CAN_H to GND: ~2.5V
   - CAN_L to GND: ~2.5V
   - CAN_H to CAN_L: ~0V
   
3. During transmission:
   - CAN_H to GND: ~3.5V (dominant)
   - CAN_L to GND: ~1.5V (dominant)
   - CAN_H to CAN_L: ~2V (dominant)
```

### Oscilloscope Setup

```
Channel 1: CAN_H (DC coupled)
Channel 2: CAN_L (DC coupled)
Math: CH1 - CH2 (differential)
Trigger: Math channel, ~1V threshold

Time base: 1-2 μs/div for 500 kbps
Voltage: 1V/div
```

### Logic Analyzer

- Connect to CAN_H and CAN_L
- Set sample rate > 10× baud rate
- Use CAN protocol decoder if available

## Common Hardware Issues

### Issue 1: No Communication

**Symptoms:** No messages, error counters increase

**Check:**
1. Transceiver power
2. Termination resistors
3. CAN_H/CAN_L polarity
4. MCU-transceiver connections

### Issue 2: Intermittent Errors

**Symptoms:** Random error frames, occasional failures

**Check:**
1. Loose connections
2. Poor grounding
3. EMI sources nearby
4. Cable quality

### Issue 3: Single Node Works, Multi-node Fails

**Symptoms:** Loopback works, external comm fails

**Check:**
1. Other nodes powered
2. All nodes same baud rate
3. Bus termination present
4. No address conflicts

### Issue 4: High Error Rate

**Symptoms:** Frequent error frames, bus-off

**Check:**
1. Signal integrity (oscilloscope)
2. Cable length vs baud rate
3. Bus loading (too many nodes)
4. Ground differences between nodes

## Cable Requirements

| Parameter | Requirement |
|-----------|-------------|
| Impedance | 120Ω characteristic |
| Type | Twisted pair |
| Shield | Optional, ground one end |
| Max length | See baud rate table |

### Baud Rate vs Cable Length

| Baud Rate | Max Length |
|-----------|------------|
| 1 Mbps | 40m |
| 500 kbps | 100m |
| 250 kbps | 250m |
| 125 kbps | 500m |
| 50 kbps | 1km |

## Connector Standards

| Type | Use Case |
|------|----------|
| DB9 | Industrial |
| OBD-II | Automotive diagnostic |
| M12 | Industrial, rugged |
| RJ45 | Custom applications |

### DB9 Pinout (Most Common)

| Pin | Signal |
|-----|--------|
| 2 | CAN_L |
| 3 | GND |
| 7 | CAN_H |
| 9 | CAN_V+ (optional) |
