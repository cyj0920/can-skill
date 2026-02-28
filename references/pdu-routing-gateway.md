# PDU路由与网关

## PduR概述

PduR (PDU Router) 负责I-PDU在不同模块之间的路由和网关功能。

## 核心功能

### 1. I-PDU路由
- 根据I-PDU ID路由到目标模块
- 支持1:1和1:N路由

### 2. 网关功能
- 不同网络之间的报文转发
- CAN ↔ CAN FD
- CAN ↔ LIN/FlexRay

### 3. 传输协议路由
- 诊断报文路由到DCM
- 大数据传输通过CanTp

## 路由路径配置

`xml
<PduRRoutingPath>
    <PduRRoutingPathId>0</PduRRoutingPathId>
    <PduRSrcPdu>
        <PduRSrcPduRef>CanIf_RxPdu_EngineData</PduRSrcPduRef>
    </PduRSrcPdu>
    <PduRDestPdu>
        <PduRDestPduRef>Com_RxPdu_EngineData</PduRDestPduRef>
        <PduRDestPduDataProvision>Immediate</PduRDestPduDataProvision>
    </PduRDestPdu>
</PduRRoutingPath>
`

## 网关场景

### 同速率网关
`
CAN1 ←→ PduR ←→ CAN2
`

### 不同速率网关
`
CAN (500kbps) ←→ PduR ←→ CAN FD (2Mbps)
`

## API接口

### 发送接口
`c
void PduR_CanIfTxConfirmation(PduIdType TxPduId, Std_ReturnType result);
void PduR_CanIfRxIndication(PduIdType RxPduId, const PduInfoType* PduInfoPtr);
`

### 路由触发
`c
void PduR_RxIndication(PduIdType RxPduId, const PduInfoType* PduInfoPtr);
void PduR_TxConfirmation(PduIdType TxPduId, Std_ReturnType result);
`
