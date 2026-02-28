# CAN 控制器状态管理

## 状态机概述

CAN控制器有四种基本状态：
- UNINIT
- STOPPED
- STARTED
- SLEEP

## 状态详细说明

### UNINIT (未初始化)
- **描述**: CAN控制器未初始化
- **寄存器状态**: 所有寄存器处于复位状态
- **中断**: CAN中断被禁用
- **总线参与**: 不参与CAN总线

### STOPPED (已停止)
- **描述**: 控制器已初始化但不参与总线
- **通信**: 逻辑链路层的错误帧不发送
- **响应**: ACK不响应
- **初始化后**: 控制器默认状态

### STARTED (已启动)
- **描述**: 控制器处于正常操作模式
- **功能**: 完整功能，参与网络
- **发送**: 可以发送报文
- **接收**: 可以接收报文

### SLEEP (睡眠)
- **描述**: 支持总线唤醒的睡眠模式
- **总线唤醒**: 可以通过CAN总线唤醒
- **无支持**: 与STOPPED状态表现一致

## 状态转换

### Can_Init()
- **转换**: UNINIT → STOPPED
- **描述**: 初始化所有CAN控制器
- **触发**: ECU启动阶段

### Can_SetControllerMode(CAN_CS_STARTED)
- **转换**: STOPPED → STARTED
- **描述**: 启动控制器参与网络
- **回调**: CanIf_ControllerModeIndication

### Can_SetControllerMode(CAN_CS_STOPPED)
- **转换**: STARTED/SLEEP → STOPPED
- **描述**: 停止控制器，取消待处理消息
- **恢复**: BusOff恢复前必须进入此状态

### 硬件事件触发

#### Hardware Wakeup
- **转换**: SLEEP → STOPPED
- **触发**: CAN总线接收到的L-PDUs
- **回调**: EcuM_CheckWakeup

#### Bus-Off
- **转换**: STARTED → STOPPED
- **触发**: TEC > 255
- **回调**: CanIf_ControllerBusOff
- **处理**: 取消待处理消息

## AUTOSAR API

### 状态设置
`c
Can_ReturnType Can_SetControllerMode(
  uint8 ControllerId,
  Can_ControllerStateType Transition
);
`

### 状态查询
`c
boolean Can_IsControllerStarted(uint8 ControllerId);
boolean Can_IsBusOff(uint8 ControllerId);
`
