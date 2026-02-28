# CanNm网络管理

## CanNm概述

CanNm (CAN Network Management) 负责协调CAN网络上ECU节点的睡眠和唤醒，实现同起同睡机制。

## 核心概念

### 唤醒定义
- **唤醒**: ECU向外发出CAN报文
- **休眠**: ECU停止向CAN总线外发出报文

### 唤醒类型
- **主动唤醒**: 自己有主动工作需求，通过网管报文唤醒其他节点
- **被动唤醒**: 接收到其他节点的网管报文，被迫唤醒

## CanNm状态机

CanNm状态机分为三种模式:
1. Bus-Sleep Mode (总线睡眠模式)
2. Network Mode (网络模式)
3. Prepare Bus-Sleep Mode (预睡眠模式)

Network Mode包含三个子状态:
- Repeat Message State
- Normal Operation State
- Ready Sleep State

### Bus-Sleep Mode
- ECU不向总线发送报文
- 刚上电初始化或准备下电前

### Repeat Message State
- 快速发送NM报文 (如20ms/帧，连续5帧)
- 告知其他节点我醒了

### Normal Operation State
- 持续发送NM报文，保持其他节点唤醒
- 有主动请求时保持在此状态

### Ready Sleep State
- 不向外发送NM报文
- 但保持应用报文发送

### Prepare Bus-Sleep Mode
- 停发应用报文和网管报文
- 等待进入Bus-Sleep Mode

## 关键时间参数

| 参数 | 描述 | 典型值 |
|------|------|--------|
| T_NM_MessageCycle | NM报文周期 | 500ms |
| T_Repeat_Message | Repeat Message状态时间 | 1500ms |
| T_NM_Timeout | NM超时时间 | 2000ms |
| T_Wait_BusSleep | 等待睡眠时间 | 5000ms |

## 常用API

### 网络请求
void CanNm_NetworkRequest(uint8 nmChannelHandle);

### 网络释放
void CanNm_NetworkRelease(uint8 nmChannelHandle);

### 获取状态
void CanNm_GetState(uint8 nmChannelHandle, Nm_StateType* nmStatePtr, Nm_ModeType* nmModePtr);
