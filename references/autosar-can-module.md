# AUTOSAR CAN 模块详解

## 模块架构概览

[Application Layer]
    ↑
  [COM / RTE]
    ↑
  [PduR]
    ↑
  [CanIf]
    ↑
  [CAN Driver (Can)]
    ↑
  [CAN Controller Hardware]

## 核心模块说明

### CAN (MCAL)
- 硬件抽象层，直接控制寄存器
- 初始化CAN控制器
- 配置波特率、邮箱
- 处理中断/轮询

### CanIf
- 接口层，统一抽象Tx/Rx接口
- 多控制器支持
- HTH/HRH管理

### PduR
- 协议数据单元路由器
- 总线间PDU路由
- 网关功能

### COM
- 通信管理
- 信号到PDU映射
- 周期性发送管理

## 关键概念

### Hardware Object (HOH)
- HTH: 发送句柄
- HRH: 接收句柄

### L-PDU
- Identifier
- Data Length (DLC)
- SDU (数据)

### Multiplexed Transmission
- 避免优先级反转
- 多硬件对象到一个HTH
- 按优先级发送

## 中断类型

1. RX中断
2. TX中断
3. BusOff中断
4. 接收FIFO中断

## 处理模式

### INTERRUPT
- 实时响应快
- 诊断响应使用

### POLLING
- 降低中断压力
- 周期报文使用
