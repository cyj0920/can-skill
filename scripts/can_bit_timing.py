#!/usr/bin/env python3
"""
CAN Bit Timing Calculator

Calculates optimal CAN bit timing parameters for a given clock frequency
and target baud rate.

Usage:
    python can_bit_timing.py --clock 36000000 --baud 500000
    python can_bit_timing.py --clock 36M --baud 500k
"""

import argparse
from dataclasses import dataclass
from typing import List, Optional


@dataclass
class TimingResult:
    """CAN bit timing configuration result."""
    prescaler: int
    total_tq: int
    ts1: int  # Time segment 1 (prop_seg + phase_seg1)
    ts2: int  # Time segment 2 (phase_seg2)
    sjw: int
    sample_point: float
    actual_baud: int
    error_percent: float

    def __str__(self) -> str:
        return (
            f"Prescaler:     {self.prescaler}\n"
            f"Total Tq:      {self.total_tq}\n"
            f"TS1 (Tq):      {self.ts1}\n"
            f"TS2 (Tq):      {self.ts2}\n"
            f"SJW:           {self.sjw}\n"
            f"Sample Point:  {self.sample_point:.1f}%\n"
            f"Actual Baud:   {self.actual_baud} bps\n"
            f"Error:         {self.error_percent:.2f}%"
        )


def parse_frequency(value: str) -> int:
    """Parse frequency string with optional suffix (K, M)."""
    value = value.upper().strip()
    multipliers = {'K': 1000, 'M': 1000000}
    
    for suffix, mult in multipliers.items():
        if value.endswith(suffix):
            return int(float(value[:-1]) * mult)
    
    return int(value)


def calculate_timing(
    clock_hz: int,
    target_baud: int,
    sample_point: float = 87.5,
    max_prescaler: int = 1024,
    min_total_tq: int = 8,
    max_total_tq: int = 25
) -> List[TimingResult]:
    """
    Calculate possible timing configurations.
    
    Args:
        clock_hz: CAN peripheral clock frequency in Hz
        target_baud: Target baud rate in bps
        sample_point: Desired sample point percentage (default 87.5%)
        max_prescaler: Maximum prescaler value
        min_total_tq: Minimum total time quanta
        max_total_tq: Maximum total time quanta
    
    Returns:
        List of valid TimingResult objects, sorted by error
    """
    results = []
    
    # Calculate ideal total Tq for target sample point
    # Sample point = (1 + TS1) / (1 + TS1 + TS2) = (1 + TS1) / Total_Tq
    
    for total_tq in range(min_total_tq, max_total_tq + 1):
        # Calculate ideal prescaler
        ideal_prescaler = clock_hz / (target_baud * total_tq)
        
        # Try prescaler values around the ideal
        for prescaler in range(max(1, int(ideal_prescaler) - 2), 
                               int(ideal_prescaler) + 3):
            if prescaler > max_prescaler or prescaler < 1:
                continue
            
            # Calculate actual baud rate
            actual_baud = clock_hz // (prescaler * total_tq)
            
            # Check if baud rate is within acceptable range
            error_percent = abs(actual_baud - target_baud) / target_baud * 100
            if error_percent > 5.0:  # Skip if error > 5%
                continue
            
            # Calculate sample point
            # Try to achieve target sample point
            ideal_ts1 = round(total_tq * sample_point / 100) - 1
            ts1 = max(1, min(16, ideal_ts1))
            ts2 = total_tq - 1 - ts1
            
            if ts2 < 1 or ts2 > 8:
                continue
            
            actual_sample_point = (1 + ts1) / total_tq * 100
            
            # SJW should be min(4, TS1, TS2)
            sjw = min(4, ts1, ts2)
            
            results.append(TimingResult(
                prescaler=prescaler,
                total_tq=total_tq,
                ts1=ts1,
                ts2=ts2,
                sjw=sjw,
                sample_point=actual_sample_point,
                actual_baud=actual_baud,
                error_percent=error_percent
            ))
    
    # Sort by baud rate error
    results.sort(key=lambda r: r.error_percent)
    return results


def generate_register_config(result: TimingResult, mcu_type: str = "stm32") -> str:
    """Generate register configuration code."""
    if mcu_type.lower() == "stm32":
        return f"""
// STM32 bxCAN register configuration
// Clock: {result.actual_baud * result.prescaler * result.total_tq} Hz
// Target baud: {result.actual_baud} bps

CAN->BTR = (({result.prescaler} - 1) << 0) |    // BRP
           (({result.ts1} - 1) << 16) |          // TS1
           (({result.ts2} - 1) << 20) |          // TS2
           (({result.sjw} - 1) << 24);           // SJW
"""
    elif mcu_type.lower() == "nxp":
        return f"""
// NXP FlexCAN register configuration
// Note: Actual register names may vary by device

CAN->CTRL1 = CAN_CTRL1_PRESDIV({result.prescaler - 1}) |
             CAN_CTRL1_PROPSEG({result.ts1 - result.ts2}) |
             CAN_CTRL1_PSEG1({result.ts2 - 1}) |
             CAN_CTRL1_PSEG2({result.ts2 - 1}) |
             CAN_CTRL1_SJW({result.sjw - 1});
"""
    else:
        return f"""
// Generic timing parameters
Prescaler:    {result.prescaler}
Time Seg 1:   {result.ts1} Tq
Time Seg 2:   {result.ts2} Tq
SJW:          {result.sjw} Tq
Total Tq:     {result.total_tq}
"""


def main():
    parser = argparse.ArgumentParser(
        description="CAN Bit Timing Calculator",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python can_bit_timing.py --clock 36000000 --baud 500000
  python can_bit_timing.py --clock 36M --baud 500k --sample 87.5
  python can_bit_timing.py --clock 48M --baud 1M --mcu stm32
        """
    )
    
    parser.add_argument(
        "--clock", "-c",
        required=True,
        help="CAN peripheral clock frequency (e.g., 36000000, 36M, 36m)"
    )
    
    parser.add_argument(
        "--baud", "-b",
        required=True,
        help="Target baud rate (e.g., 500000, 500k, 1M)"
    )
    
    parser.add_argument(
        "--sample", "-s",
        type=float,
        default=87.5,
        help="Target sample point percentage (default: 87.5)"
    )
    
    parser.add_argument(
        "--mcu", "-m",
        choices=["stm32", "nxp", "generic"],
        default="stm32",
        help="MCU type for code generation (default: stm32)"
    )
    
    parser.add_argument(
        "--all", "-a",
        action="store_true",
        help="Show all valid configurations"
    )
    
    args = parser.parse_args()
    
    # Parse input values
    clock_hz = parse_frequency(args.clock)
    target_baud = parse_frequency(args.baud)
    
    print(f"CAN Bit Timing Calculator")
    print(f"=" * 40)
    print(f"Clock Frequency: {clock_hz:,} Hz")
    print(f"Target Baud Rate: {target_baud:,} bps")
    print(f"Target Sample Point: {args.sample}%")
    print()
    
    # Calculate timing options
    results = calculate_timing(
        clock_hz,
        target_baud,
        args.sample
    )
    
    if not results:
        print("ERROR: No valid timing configuration found!")
        print("Try a different clock frequency or baud rate.")
        return 1
    
    # Display results
    if args.all:
        print(f"Found {len(results)} valid configuration(s):\n")
        for i, result in enumerate(results[:10], 1):  # Show top 10
            print(f"--- Option {i} ---")
            print(result)
            print()
    else:
        # Show only the best result
        print("Best Configuration:")
        print("-" * 20)
        print(results[0])
        print()
    
    # Generate code for best result
    print("Register Configuration:")
    print("-" * 20)
    print(generate_register_config(results[0], args.mcu))
    
    # Validation info
    best = results[0]
    if best.error_percent > 1.0:
        print(f"WARNING: Baud rate error is {best.error_percent:.2f}%")
        print("Consider using a different clock frequency for better accuracy.")
    
    if abs(best.sample_point - args.sample) > 2.0:
        print(f"NOTE: Sample point is {best.sample_point:.1f}% (target: {args.sample}%)")
    
    return 0


if __name__ == "__main__":
    exit(main())
