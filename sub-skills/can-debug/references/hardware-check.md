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

## Termination Methods

### Standard Termination

```
     120Ω              120Ω
  ┌──/\/\/\──┐     ┌──/\/\/\──┐
  │          │     │          │
CAN_H       CAN_H CAN_L       CAN_L
  │          │     │          │
  └──────────┴─────┴──────────┘
        BUS (120Ω characteristic)
```

### Split Termination (Improved EMI)

```
          ┌── 60Ω ──┬── 60Ω ──┐
          │         │         │
      CAN_H         │        CAN_L
                    │
                   4.7nF
                    │
                   GND
```

**Benefits**:
- Better common mode filtering
- Reduced EMI emissions
- Common mode stabilization

### Split Pin (Common Mode Stabilization)

Many transceivers have a SPLIT pin for this purpose:
- TJA1042, TJA1043, TJA1055
- Connect SPLIT to center tap of split termination
- Provides stable common mode voltage (~2.5V)

## Common Mode Choke

### Purpose

A common mode choke:
- Passes differential signals (low impedance)
- Blocks common mode noise (high impedance)
- Reduces EMI emissions and improves immunity

### Selection Criteria

| Parameter | Typical Value | Notes |
|-----------|---------------|-------|
| Inductance | 25-100 µH | Higher = better CM filtering |
| DC Resistance | < 1Ω | Lower = less signal loss |
| Current Rating | > 100mA | Must handle bus current |
| Common Mode Impedance | > 100Ω @ 100MHz | EMI suppression |

### Common Part Numbers

| Part | Inductance | DCR | Package |
|------|------------|-----|---------|
| B82789 | 2×51µH | 0.1Ω | SMD |
| DLW43SH101 | 100µH | 0.3Ω | SMD |
| WE-CNSW | 2×47µH | 0.12Ω | SMD |

### Placement

```
MCU ── TXD/RXD ── Transceiver ── CMC ── CAN Bus
                              │
                           ESD Protection
```

**Rules**:
- Place as close to transceiver as possible
- After ESD protection diodes
- Before bus termination

## ESD and Transient Protection

### Components

| Component | Purpose | Placement |
|-----------|---------|-----------|
| TVS diodes | ESD/fast transients | At connector |
| Common mode choke | EMI filtering | After TVS |
| Series resistors | Current limiting | At transceiver |

### Typical Circuit

```
                    ┌─────────────────┐
CAN_H ──┬──[TVS]──┬─┤ CMC      CMC    ├──┬── 120Ω termination
        │         │ └─────────────────┘  │
        │         │                      │
        └─────────┴── To Transceiver     │
                                       CAN Bus
```

## Shielding and Grounding

### Shielded Cable

| Cable Type | Shield Type | Application |
|------------|-------------|-------------|
| Standard CAN | Unshielded | Low EMI environment |
| Industrial CAN | Foil + Braid | High EMI environment |
| Automotive | Foil shield | Weight sensitive |

### Shield Termination

```
Option 1: Single-point ground (preferred for low frequency)
  Shield ───┬── GND at one end only
            │
          360° clamp at connector

Option 2: Capacitive ground (for high frequency)
  Shield ───┬── GND via 1nF-10nF capacitor
            │
          360° clamp at connector
```

### Grounding Rules

1. **Never** connect shield at both ends directly (ground loop)
2. Use 360° shield clamp at connector
3. Keep ground leads short
4. Use common mode choke for additional isolation

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

### Signal Quality Measurements

| Parameter | Good | Marginal | Poor |
|-----------|------|----------|------|
| Differential amplitude | 1.5-3V | 1.2-1.5V | < 1.2V |
| Rise time | 20-50ns | 50-100ns | > 100ns |
| Overshoot | < 10% | 10-20% | > 20% |
| Ringing | Minimal | Moderate | Severe |

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

### Issue 5: EMI-Related Failures

**Symptoms:** Errors near motors, solenoids, relays

**Check:**
1. Shield grounding
2. Common mode choke presence
3. Cable routing near noise sources
4. Transient protection

## Cable Requirements

| Parameter | Requirement |
|-----------|-------------|
| Impedance | 120Ω characteristic |
| Type | Twisted pair |
| Shield | Optional, ground one end |
| Max length | See baud rate table |

### Baud Rate vs Cable Length

| Baud Rate | Max Length | Notes |
|-----------|------------|-------|
| 1 Mbps | 40m | Standard limit |
| 500 kbps | 100m | Most common |
| 250 kbps | 250m | |
| 125 kbps | 500m | |
| 50 kbps | 1km | Use repeaters for longer |

### Cable Selection Guide

| Environment | Cable Type | Shield |
|-------------|------------|--------|
| Office/Lab | Standard twisted pair | Optional |
| Industrial | Industrial CAN cable | Yes |
| Automotive | Automotive grade | Yes |
| Outdoor | UV-resistant jacket | Yes |

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

## EMI Troubleshooting Checklist

```
□ Check for noise sources near CAN cable
□ Verify shield is properly grounded
□ Add common mode choke if missing
□ Check for ground loops between nodes
□ Use twisted pair cable
□ Separate CAN cable from power cables
□ Add TVS diodes at connector
□ Consider ferrite beads on cable
```