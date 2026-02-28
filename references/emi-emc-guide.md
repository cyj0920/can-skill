# EMI/EMC Guide for CAN Bus

## Overview

CAN bus operates in electrically noisy environments (automotive, industrial).
Proper EMI/EMC design is critical for reliable communication.

## Common EMI Sources

### Internal Sources

| Source | Frequency | Coupling Mechanism |
|--------|-----------|-------------------|
| Switching power supplies | 100kHz-1MHz | Conducted/Radiated |
| MCU clock oscillators | 8-100MHz | Radiated |
| Motor drivers | DC-100kHz | Magnetic coupling |
| Relay contacts | Transients | Contact arcing |
| LCD backlights | 20-100kHz | Conducted |

### External Sources

| Source | Characteristics |
|--------|-----------------|
| Electric motors | Broadband noise, transients |
| Welding equipment | High current transients |
| Radio transmitters | Narrowband RF |
| Lightning | High energy transients |
| Power lines | 50/60Hz + harmonics |

## EMI Coupling Paths

```
Noise Source → Coupling Path → CAN Bus

Coupling Mechanisms:
1. Conducted: Through power/ground connections
2. Capacitive: Electric field coupling
3. Inductive: Magnetic field coupling
4. Radiated: Electromagnetic waves
```

## Prevention Strategies

### 1. Cable Design

**Twisted Pair Benefits**:
- Cancels magnetic field pickup
- Reduces radiated emissions
- Standard: 1 twist per 10-20mm

**Shielding Effectiveness**:

| Shield Type | Effectiveness | Use Case |
|-------------|---------------|----------|
| Unshielded | 0 dB | Low EMI only |
| Foil shield | 40-60 dB | Moderate EMI |
| Braid shield | 60-80 dB | High EMI |
| Foil + Braid | 80-100 dB | Severe EMI |

### 2. Common Mode Filtering

**Why Common Mode?**
- Differential signals: Desired communication
- Common mode noise: Unwanted interference

**Common Mode Choke Placement**:
```
Transceiver ── CMC ── Connector ── Cable
              │
          [Optional: ESD protection]
```

**Selection Guide**:

| Application | Inductance | Part Example |
|-------------|------------|--------------|
| General purpose | 51µH | WE-CNSW 744232090 |
| Automotive | 100µH | DLW43SH101XK2 |
| High speed CAN-FD | 25µH | ACM2012-900-2P |

### 3. Grounding Strategy

**Ground Rules**:

1. **Single Point Ground** for low frequency (< 1MHz):
   - Connect all grounds at one point
   - Prevents ground loops

2. **Multi-Point Ground** for high frequency (> 10MHz):
   - Connect grounds at multiple points
   - Lower impedance at high frequency

3. **Shield Grounding**:
   - Ground shield at ONE end only (typically controller end)
   - Use 360° clamp at connector
   - For high frequency: capacitive coupling (1-10nF)

**Ground Loop Prevention**:
```
Node A ←────────────────→ Node B
  │                        │
  ↓                        ↓
Ground A ←─ Resistance ─→ Ground B
           (ground loop)

Solution: Isolate one node or use common ground
```

### 4. Transient Protection

**TVS Diode Selection**:

| Parameter | Requirement |
|-----------|-------------|
| Working voltage | > 5V (CAN bus voltage) |
| Clamping voltage | < 40V (transceiver max) |
| Peak power | > 200W typical |
| Capacitance | < 50pF (for high speed) |

**Recommended Parts**:

| Part | V_RWM | V_CL | Package |
|------|-------|------|---------|
| PESD2CAN | 24V | 40V | SOT-23 |
| NUP2105 | 24V | 45V | SOT-23 |
| SM712 | 12V | 26V | SOT-23 |

### 5. PCB Layout

**Critical Layout Rules**:

1. Keep CAN signals short and direct
2. Place CMC close to transceiver
3. Place TVS close to connector
4. Separate CAN traces from noisy signals
5. Use solid ground plane under CAN circuitry
6. Minimize loop areas in CAN signal paths

**Example Layout**:
```
┌─────────────────────────────────────┐
│  MCU                                │
│    │                                │
│   TXD/RXD                           │
│    │                                │
│  ┌──┴──┐                           │
│  │XCVR │ ← Keep close to MCU        │
│  └──┬──┘                           │
│     │                              │
│   ┌─┴─┐                            │
│   │CMC│ ← Common mode choke         │
│   └─┬─┘                            │
│     │                              │
│   ┌─┴─┐                            │
│   │TVS│ ← ESD protection            │
│   └─┬─┘                            │
│     │                              │
│  ┌──┴──┐                           │
│  │CONN │ ← Connector                │
│  └─────┘                           │
└─────────────────────────────────────┘
```

## EMI Testing

### Pre-compliance Tests

**1. Near-field scanning**:
- Use near-field probe
- Scan PCB and cables
- Identify noise sources

**2. Current probe test**:
- Clamp current probe on CAN cable
- Measure common mode current
- Should be < 10µA typical

**3. Voltage probe test**:
- Measure CAN_H and CAN_L individually
- Check for noise spikes
- Verify signal levels

### Emission Limits (CISPR 25)

| Frequency | Limit (Class 5) |
|-----------|-----------------|
| 150kHz-30MHz | 50-40 dBµV/m |
| 30MHz-200MHz | 40-30 dBµV/m |
| 200MHz-1GHz | 30-25 dBµV/m |

### Immunity Tests (ISO 11452)

| Test | Level | Requirement |
|------|-------|-------------|
| BCI | 200mA | No errors |
| Radiated | 100V/m | No errors |
| Transient | ±100V | Recovery |

## Troubleshooting EMI Issues

### Step 1: Identify Symptoms

| Symptom | Likely Cause |
|---------|--------------|
| Random bit errors | EMI coupling |
| Errors at specific times | Periodic noise source |
| Errors near motor/relay | Magnetic coupling |
| Errors at high speed | Insufficient filtering |

### Step 2: Locate Noise Source

1. Use spectrum analyzer with near-field probe
2. Identify frequency of interference
3. Correlate with system components

### Step 3: Apply Mitigation

**Quick Fixes**:
1. Add ferrite bead on cable near connector
2. Improve grounding connections
3. Add bypass capacitors at transceiver

**Permanent Solutions**:
1. Add common mode choke
2. Install shielded cable
3. Improve PCB layout
4. Add transient protection

## Design Checklist

```
□ Twisted pair cable used
□ 120Ω termination at both ends
□ Common mode choke installed
□ TVS/ESD protection at connector
□ Proper shield grounding (one end)
□ Solid ground plane under CAN
□ CAN traces separated from noise sources
□ Transceiver close to connector
□ Tested with expected noise sources
□ Meets EMI/EMC requirements
```

## Quick Reference

| Problem | Solution |
|---------|----------|
| High emissions | Add CMC, shield cable |
| Low immunity | Add TVS, improve grounding |
| Ground loop | Single-point ground or isolation |
| Magnetic pickup | Use twisted pair, move away from source |
| Radiated pickup | Shield cable, improve filtering |
