# CAN Skill

[English](#english) | [中文](#中文)

---

<a name="english"></a>

## English

### Overview

A comprehensive AI Agent Skill for embedded MCU CAN/CAN-FD bus development, covering driver implementation, debugging, diagnosis, and testing.

### Features

- **Driver Development**: CAN/CAN-FD initialization, TX/RX, interrupt handling, filter configuration
- **Debug & Analysis**: Signal quality analysis, timing debug, hardware inspection
- **Fault Diagnosis**: Error frame analysis, bus state monitoring, troubleshooting guide
- **Testing**: Loopback test, stress test, test pattern generation

### Supported Platforms

| MCU Family | Support Level |
|------------|---------------|
| STM32 (F1/F4/H7) | Full support with examples |
| NXP (S32K/LPC) | Full support with examples |
| Infineon (Aurix/TC) | Generic register-level code |
| Other MCUs | Generic templates provided |

### Quick Start

1. Invoke this skill when you need help with CAN bus development
2. Specify your target MCU platform
3. Describe your task (driver dev, debug, diagnosis, or testing)

### Documentation

| Document | Description |
|----------|-------------|
| `references/can-protocol.md` | CAN protocol fundamentals |
| `references/can-fd-extension.md` | CAN-FD extensions and migration |
| `references/emi-emc-guide.md` | EMI/EMC design guide |
| `references/common-fault-cases.md` | 10 real-world fault cases |

### Quick Diagnosis

**Over 80% of CAN issues are physical layer problems:**

1. Check termination: CAN_H to CAN_L should be ~60Ω
2. Check idle voltage: CAN_H and CAN_L should be ~2.5V each
3. Check baud rate: All nodes must match (< 0.5% tolerance)
4. Check wiring: CAN_H to CAN_H, CAN_L to CAN_L

### Directory Structure

```
can-skill/
├── SKILL.md                    # Main entry point
├── sub-skills/
│   ├── can-driver-dev/         # Driver development
│   ├── can-debug/              # Debugging analysis
│   ├── can-diagnosis/          # Problem diagnosis
│   └── can-testing/            # Testing verification
├── references/                 # Technical documentation
└── scripts/                    # Python tools
```

### Version

Current version: **v1.1.0**

---

<a name="中文"></a>

## 中文

### 概述

一个全面的AI Agent技能包，用于嵌入式MCU CAN/CAN-FD总线开发，涵盖驱动开发、调试分析、故障诊断和测试验证。

### 功能特性

- **驱动开发**：CAN/CAN-FD初始化、收发、中断处理、滤波器配置
- **调试分析**：信号质量分析、时序调试、硬件检查
- **故障诊断**：错误帧分析、总线状态监控、故障排查指南
- **测试验证**：回环测试、压力测试、测试模式生成

### 支持平台

| MCU系列 | 支持程度 |
|---------|----------|
| STM32 (F1/F4/H7) | 完整支持，含示例代码 |
| NXP (S32K/LPC) | 完整支持，含示例代码 |
| Infineon (Aurix/TC) | 通用寄存器级代码 |
| 其他MCU | 提供通用模板 |

### 快速开始

1. 当你需要CAN总线开发帮助时调用此技能
2. 指定你的目标MCU平台
3. 描述你的任务（驱动开发、调试、诊断或测试）

### 文档

| 文档 | 描述 |
|------|------|
| `references/can-protocol.md` | CAN协议基础 |
| `references/can-fd-extension.md` | CAN-FD扩展与迁移 |
| `references/emi-emc-guide.md` | EMI/EMC设计指南 |
| `references/common-fault-cases.md` | 10个真实故障案例 |

### 快速诊断

**超过80%的CAN问题是物理层问题：**

1. 检查终端电阻：CAN_H到CAN_L应为~60Ω
2. 检查空闲电压：CAN_H和CAN_L应各为~2.5V
3. 检查波特率：所有节点必须匹配（容差< 0.5%）
4. 检查接线：CAN_H接CAN_H，CAN_L接CAN_L

### 目录结构

```
can-skill/
├── SKILL.md                    # 主入口
├── sub-skills/
│   ├── can-driver-dev/         # 驱动开发
│   ├── can-debug/              # 调试分析
│   ├── can-diagnosis/          # 故障诊断
│   └── can-testing/            # 测试验证
├── references/                 # 技术文档
└── scripts/                    # Python工具
```

### 版本

当前版本：**v1.1.0**

---

## License

MIT License

## Author

cyj0920
