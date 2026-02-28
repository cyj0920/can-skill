#!/usr/bin/env python3
"""
CAN Message Analyzer

Analyzes CAN message logs and provides statistics, error detection,
and pattern recognition.

Usage:
    python can_analyzer.py --input logfile.txt
    python can_analyzer.py --input logfile.txt --format csv
"""

import argparse
import re
from collections import defaultdict
from dataclasses import dataclass
from datetime import datetime
from typing import List, Dict, Optional
from enum import Enum


class FrameType(Enum):
    DATA = "DATA"
    REMOTE = "REMOTE"
    ERROR = "ERROR"
    OVERLOAD = "OVERLOAD"


@dataclass
class CANFrame:
    """Represents a CAN frame."""
    timestamp: float
    id: int
    extended: bool
    frame_type: FrameType
    dlc: int
    data: bytes
    
    def __str__(self) -> str:
        id_str = f"{self.id:08X}" if self.extended else f"{self.id:03X}"
        data_str = " ".join(f"{b:02X}" for b in self.data[:self.dlc])
        type_str = "R" if self.frame_type == FrameType.REMOTE else "D"
        return f"{self.timestamp:.6f}  {id_str}  [{self.dlc}]  {data_str}  ({type_str})"


@dataclass
class AnalysisResult:
    """Analysis results for a CAN log."""
    total_frames: int
    data_frames: int
    remote_frames: int
    error_frames: int
    unique_ids: int
    bus_load_percent: float
    avg_message_rate: float
    id_statistics: Dict[int, Dict]
    data_patterns: Dict[int, List[bytes]]
    errors: List[str]


def parse_log_line(line: str) -> Optional[CANFrame]:
    """
    Parse a single log line.
    
    Supports multiple formats:
    - Vector CANoe: timestamp ID DIR DLC DATA
    - SocketCAN: (timestamp) interface ID#DATA
    - Simple: timestamp ID DLC DATA
    """
    line = line.strip()
    if not line or line.startswith("#"):
        return None
    
    # Try Vector CANoe format
    # 0.000000  123  Rx  d 8 00 01 02 03 04 05 06 07
    vector_pattern = r"(\d+\.\d+)\s+(\w+)\s+\w+\s+[dDrR]\s*(\d)\s*([0-9A-Fa-f\s]*)"
    match = re.match(vector_pattern, line)
    if match:
        timestamp = float(match.group(1))
        can_id = int(match.group(2), 16)
        dlc = int(match.group(3))
        data_str = match.group(4).strip()
        data = bytes.fromhex(data_str.replace(" ", ""))
        return CANFrame(
            timestamp=timestamp,
            id=can_id,
            extended=can_id > 0x7FF,
            frame_type=FrameType.DATA,
            dlc=dlc,
            data=data
        )
    
    # Try SocketCAN format
    # (000.000000) can0 123#0102030405060708
    socketcan_pattern = r"\((\d+\.\d+)\)\s+\w+\s+([0-9A-Fa-f]+)#([0-9A-Fa-f]*)"
    match = re.match(socketcan_pattern, line)
    if match:
        timestamp = float(match.group(1))
        can_id = int(match.group(2), 16)
        data_str = match.group(3)
        data = bytes.fromhex(data_str) if data_str else bytes()
        return CANFrame(
            timestamp=timestamp,
            id=can_id,
            extended=can_id > 0x7FF,
            frame_type=FrameType.DATA,
            dlc=len(data),
            data=data
        )
    
    # Try simple format
    # 0.000000 123 8 00 01 02 03 04 05 06 07
    simple_pattern = r"(\d+\.\d+)\s+([0-9A-Fa-f]+)\s+(\d)\s*([0-9A-Fa-f\s]*)"
    match = re.match(simple_pattern, line)
    if match:
        timestamp = float(match.group(1))
        can_id = int(match.group(2), 16)
        dlc = int(match.group(3))
        data_str = match.group(4).strip()
        data = bytes.fromhex(data_str.replace(" ", "")) if data_str else bytes()
        return CANFrame(
            timestamp=timestamp,
            id=can_id,
            extended=can_id > 0x7FF,
            frame_type=FrameType.DATA,
            dlc=dlc,
            data=data
        )
    
    return None


def analyze_frames(frames: List[CANFrame]) -> AnalysisResult:
    """Analyze a list of CAN frames."""
    if not frames:
        return AnalysisResult(
            total_frames=0,
            data_frames=0,
            remote_frames=0,
            error_frames=0,
            unique_ids=0,
            bus_load_percent=0,
            avg_message_rate=0,
            id_statistics={},
            data_patterns={},
            errors=[]
        )
    
    # Count frame types
    data_frames = sum(1 for f in frames if f.frame_type == FrameType.DATA)
    remote_frames = sum(1 for f in frames if f.frame_type == FrameType.REMOTE)
    error_frames = sum(1 for f in frames if f.frame_type == FrameType.ERROR)
    
    # Calculate timing
    duration = frames[-1].timestamp - frames[0].timestamp
    avg_message_rate = len(frames) / duration if duration > 0 else 0
    
    # Estimate bus load (rough calculation)
    # Average frame: ~110 bits for standard frame with 8 data bytes
    avg_bits_per_frame = 110
    total_bits = len(frames) * avg_bits_per_frame
    bus_load_percent = (total_bits / (duration * 500000)) * 100 if duration > 0 else 0
    
    # ID statistics
    id_stats = defaultdict(lambda: {
        "count": 0,
        "dlc_min": 8,
        "dlc_max": 0,
        "dlc_avg": 0,
        "first_seen": float("inf"),
        "last_seen": 0,
        "interval_avg": 0,
        "intervals": []
    })
    
    data_patterns = defaultdict(list)
    
    for frame in frames:
        stats = id_stats[frame.id]
        stats["count"] += 1
        stats["dlc_min"] = min(stats["dlc_min"], frame.dlc)
        stats["dlc_max"] = max(stats["dlc_max"], frame.dlc)
        stats["first_seen"] = min(stats["first_seen"], frame.timestamp)
        stats["last_seen"] = max(stats["last_seen"], frame.timestamp)
        
        # Track intervals
        if "last_timestamp" in stats:
            interval = frame.timestamp - stats["last_timestamp"]
            stats["intervals"].append(interval)
        stats["last_timestamp"] = frame.timestamp
        
        # Store data patterns (first 4 bytes)
        if frame.dlc >= 4:
            data_patterns[frame.id].append(frame.data[:4])
    
    # Calculate averages
    for can_id, stats in id_stats.items():
        if stats["intervals"]:
            stats["interval_avg"] = sum(stats["intervals"]) / len(stats["intervals"])
        stats["dlc_avg"] = stats["dlc_min"]  # Simplified
    
    # Detect errors
    errors = []
    
    # Check for missing messages (gaps)
    if duration > 1.0 and avg_message_rate > 0:
        expected_frames = int(avg_message_rate * duration)
        if len(frames) < expected_frames * 0.9:
            errors.append(f"Possible message loss: expected ~{expected_frames}, got {len(frames)}")
    
    # Check for ID conflicts
    for can_id, stats in id_stats.items():
        if stats["dlc_min"] != stats["dlc_max"]:
            errors.append(f"ID {can_id:03X} has varying DLC: {stats['dlc_min']}-{stats['dlc_max']}")
    
    return AnalysisResult(
        total_frames=len(frames),
        data_frames=data_frames,
        remote_frames=remote_frames,
        error_frames=error_frames,
        unique_ids=len(id_stats),
        bus_load_percent=min(100, bus_load_percent),
        avg_message_rate=avg_message_rate,
        id_statistics=dict(id_stats),
        data_patterns=dict(data_patterns),
        errors=errors
    )


def print_analysis(result: AnalysisResult):
    """Print analysis results."""
    print("\n" + "=" * 50)
    print("CAN Log Analysis Report")
    print("=" * 50)
    
    print(f"\n[General Statistics]")
    print(f"  Total Frames:      {result.total_frames}")
    print(f"  Data Frames:       {result.data_frames}")
    print(f"  Remote Frames:     {result.remote_frames}")
    print(f"  Error Frames:      {result.error_frames}")
    print(f"  Unique IDs:        {result.unique_ids}")
    print(f"  Avg Message Rate:  {result.avg_message_rate:.1f} msg/s")
    print(f"  Estimated Load:    {result.bus_load_percent:.1f}%")
    
    if result.id_statistics:
        print(f"\n[ID Statistics]")
        print(f"  {'ID':>8}  {'Count':>8}  {'DLC':>6}  {'Interval (ms)':>15}")
        print(f"  {'-'*8}  {'-'*8}  {'-'*6}  {'-'*15}")
        
        # Sort by count
        sorted_ids = sorted(
            result.id_statistics.items(),
            key=lambda x: x[1]["count"],
            reverse=True
        )
        
        for can_id, stats in sorted_ids[:20]:  # Top 20 IDs
            id_str = f"{can_id:08X}" if can_id > 0x7FF else f"{can_id:03X}"
            interval_ms = stats["interval_avg"] * 1000 if stats["interval_avg"] > 0 else 0
            print(f"  {id_str:>8}  {stats['count']:>8}  {stats['dlc_min']:>6}  {interval_ms:>15.2f}")
    
    if result.errors:
        print(f"\n[Warnings]")
        for error in result.errors:
            print(f"  - {error}")
    
    print("\n" + "=" * 50)


def main():
    parser = argparse.ArgumentParser(
        description="CAN Message Analyzer",
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    
    parser.add_argument(
        "--input", "-i",
        required=True,
        help="Input log file path"
    )
    
    parser.add_argument(
        "--format", "-f",
        choices=["auto", "vector", "socketcan", "simple"],
        default="auto",
        help="Log file format (default: auto-detect)"
    )
    
    parser.add_argument(
        "--stats-only", "-s",
        action="store_true",
        help="Show only summary statistics"
    )
    
    args = parser.parse_args()
    
    # Read and parse log file
    frames = []
    try:
        with open(args.input, "r") as f:
            for line in f:
                frame = parse_log_line(line)
                if frame:
                    frames.append(frame)
    except FileNotFoundError:
        print(f"ERROR: File not found: {args.input}")
        return 1
    except Exception as e:
        print(f"ERROR reading file: {e}")
        return 1
    
    if not frames:
        print("ERROR: No valid CAN frames found in log file")
        return 1
    
    # Analyze
    result = analyze_frames(frames)
    
    # Print results
    print_analysis(result)
    
    return 0


if __name__ == "__main__":
    exit(main())
