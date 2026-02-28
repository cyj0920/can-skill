# CanTp传输协议

## CanTp概述

CanTp (CAN Transport Protocol) 负责CAN I-PDU的分段和重组，实现大于8字节的数据传输。

## 四种帧类型

### SF (Single Frame) - 单帧
- 用于传输不超过7字节的数据
- PCI字节包含数据长度

### FF (First Frame) - 首帧
- 多帧传输的第一帧
- 包含完整数据长度信息

### CF (Consecutive Frame) - 连续帧
- 首帧之后的后续帧
- 包含序列号(SN)

### FC (Flow Control) - 流控帧
- 调节发送速率
- 包含FS/BS/STmin参数

## 流控参数

| 参数 | 描述 |
|------|------|
| FS (Flow Status) | 流控状态: CTS/WAIT/OVFLW |
| BS (Block Size) | 块大小，允许发送的CF数量 |
| STmin | 最小间隔时间 |

## 传输流程

### 单帧传输
`
发送方: SF [数据]
接收方: 直接接收完成
`

### 多帧传输
`
发送方: FF [总长度][数据]
接收方: FC [FS=CTS][BS][STmin]
发送方: CF [SN=1][数据]
发送方: CF [SN=2][数据]
...
接收方: 接收完成确认
`

## 寻址格式

- **Normal**: 标准寻址
- **Extended**: 扩展寻址 (含N_TA)
- **Mixed 11-bit**: 混合11位寻址
- **Normal fixed**: 固定寻址
- **Mixed 29-bit**: 混合29位寻址
