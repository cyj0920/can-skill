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
| `references/can-protocol.md` | CAN protocol fundamentals, loop/open-loop, bit timing |
| `references/can-fd-extension.md` | CAN-FD extensions and migration |
| `references/emi-emc-guide.md` | EMI/EMC design guide |
| `references/common-fault-cases.md` | 10 real-world fault cases |
| `references/can-buffer-mode.md` | Basic CAN vs Full CAN, FIFO configuration |
| `references/autosar-can-module.md` | AUTOSAR CAN stack architecture |
| `references/can-priority-inversion.md` | Priority inversion analysis and solutions |
| `references/can-controller-states.md` | Controller state machine management |
| `references/can-engineering-practices.md` | Engineering best practices |
| `references/can-mcal-basics.md` | CAN MCAL register configuration |
| `references/uds-diagnostic-services.md` | UDS 0x22/0x2E/0x31 services |
| `references/cannm-network-management.md` | CanNm sleep/wakeup state machine |
| `references/cantp-transport-protocol.md` | SF/FF/CF/FC frame handling |
| `references/pdu-routing-gateway.md` | PDU routing and gateway configuration |
| `sub-skills/can-diagnosis/references/busoff-recovery.md` | Busoff recovery mechanisms |

### Quick Diagnosis

**Physical Layer (Basic Check):**
1. Check termination: CAN_H to CAN_L should be ~60Ω
2. Check idle voltage: CAN_H and CAN_L should be ~2.5V each
3. Check baud rate: All nodes must match (< 0.5% tolerance)

**Software Layer (AUTOSAR/Common Issues):**
1. **Priority Inversion**: Check FULL vs BASIC CAN configuration
2. **BusOff Recovery**: Verify CanBusOffRecovery settings and state transitions
3. **FIFO Overflow**: Check FIFO depth vs message reception rate
4. **State Machine**: Verify controller state transitions (STOPPED → STARTED)
5. **Message Delay**: Check HTH/HRH configuration and interrupt/polling mode

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

Current version: **v1.4.0**

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
| `references/can-protocol.md` | CAN协议基础、闭环/开环结构、位时序 |
| `references/can-fd-extension.md` | CAN-FD扩展与迁移 |
| `references/emi-emc-guide.md` | EMI/EMC设计指南 |
| `references/common-fault-cases.md` | 10个真实故障案例 |
| `references/can-buffer-mode.md` | Basic CAN vs Full CAN、FIFO配置 |
| `references/autosar-can-module.md` | AUTOSAR CAN栈架构详解 |
| `references/can-priority-inversion.md` | 优先级反转分析与解决方案 |
| `references/can-controller-states.md` | 控制器状态机管理 |
| `references/can-engineering-practices.md` | 工程实践最佳实践 |
| `references/can-mcal-basics.md` | CAN MCAL寄存器配置 |
| `references/uds-diagnostic-services.md` | UDS 0x22/0x2E/0x31服务实现 |
| `references/cannm-network-management.md` | CanNm睡眠唤醒状态机 |
| `references/cantp-transport-protocol.md` | SF/FF/CF/FC帧处理 |
| `references/pdu-routing-gateway.md` | PDU路由与网关配置 |
| `sub-skills/can-diagnosis/references/busoff-recovery.md` | Busoff恢复机制 |

### 快速诊断

**物理层（基础检查）：**
1. 检查终端电阻：CAN_H到CAN_L应为~60Ω
2. 检查空闲电压：CAN_H和CAN_L应各为~2.5V
3. 检查波特率：所有节点必须匹配（容差< 0.5%）

**软件层（AUTOSAR/常见问题）：**
1. **优先级反转**: 检查FULL vs BASIC CAN配置
2. **BusOff恢复**: 验证CanBusOffRecovery设置和状态转换
3. **FIFO溢出**: 检查FIFO深度与报文接收频率匹配
4. **状态机异常**: 验证控制器状态转换（STOPPED → STARTED）
5. **报文延迟**: 检查HTH/HRH配置和中断/轮询模式

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

当前版本：**v1.4.0**

---

## License

MIT License

## Author

cyj0920
