# UDS诊断服务实现

## UDS概述

UDS (Unified Diagnostic Services) 统一诊断服务，基于ISO 14229标准。

### DCM模块架构

`
[Tester]
    ↑
[CanTp]  ← 传输层
    ↑
[DCM]    ← 诊断通信管理 (DSL/DSD/DSP)
    ↑
[Application/DEM/NvM]
`

DCM三个子模块:
- **DSL (Diagnostic Session Layer)**: 会话管理
- **DSD (Diagnostic Service Dispatcher)**: 服务分发
- **DSP (Diagnostic Service Processing)**: 服务处理

## 0x22 ReadDataByIdentifier

### 功能描述
通过数据标识符(DID)读取ECU中的数据。

### 请求格式
| Byte | 描述 |
|------|------|
| 0    | SID = 0x22 |
| 1-2  | DID (Data Identifier) |

### 肯定响应
| Byte | 描述 |
|------|------|
| 0    | SID + 0x40 = 0x62 |
| 1-2  | DID |
| 3-n  | Data |

### 否定响应
| Byte | 描述 |
|------|------|
| 0    | 0x7F |
| 1    | SID = 0x22 |
| 2    | NRC (Negative Response Code) |

### 常用NRC
- 0x31: requestSequenceError (请求顺序错误)
- 0x78: responsePending (响应等待)

### DID配置

#### 静态DID
`c
// DID映射到变量
const Dcm_DidConfigType Dcm_DidConfig[] = {
    {
        .Did = 0xF190,  // VIN码
        .DataLength = 17,
        .ReadFunc = App_ReadVIN,
        .WriteFunc = NULL
    },
    {
        .Did = 0xF193,  // 供应商硬件版本
        .DataLength = 4,
        .ReadFunc = App_ReadHWVersion,
        .WriteFunc = NULL
    }
};
`

#### NvM映射
`c
// DID映射到NvM Block
const Dcm_DidNvMConfigType Dcm_DidNvMConfig[] = {
    {
        .Did = 0x0100,
        .NvMBlockId = NVM_BLOCK_ID_CONFIG_DATA,
        .NvMReadFunc = NvM_ReadBlock,
        .NvMWriteFunc = NvM_WriteBlock
    }
};
`

## 0x2E WriteDataByIdentifier

### 功能描述
通过DID写入数据到ECU。

### 请求格式
| Byte | 描述 |
|------|------|
| 0    | SID = 0x2E |
| 1-2  | DID |
| 3-n  | Data |

### 肯定响应
| Byte | 描述 |
|------|------|
| 0    | SID + 0x40 = 0x6E |
| 1-2  | DID |

### 安全访问要求
写DID通常需要先通过0x27安全访问服务解锁。

## 0x31 RoutineControl

### 功能描述
控制例行程序(如擦除Flash、自检等)。

### 子功能
- 0x01: startRoutine (启动例行程序)
- 0x02: stopRoutine (停止例行程序)
- 0x03: requestRoutineResults (请求结果)

### 请求格式
| Byte | 描述 |
|------|------|
| 0    | SID = 0x31 |
| 1    | Sub-function |
| 2-3  | RID (Routine Identifier) |
| 4-n  | Routine Control Option Record |

### 肯定响应
| Byte | 描述 |
|------|------|
| 0    | SID + 0x40 = 0x71 |
| 1    | Sub-function |
| 2-3  | RID |
| 4-n  | Routine Info / Result |

### 异步处理

#### 同步执行
`c
Std_ReturnType App_Routine_EraseFlash(uint8* request, uint8* response)
{
    // 直接执行并返回结果
    Flash_EraseSector(request[0]);
    return E_OK;
}
`

#### 异步执行 (Pending)
`c
Std_ReturnType App_Routine_Start(uint8* request, uint8* response)
{
    // 启动例行程序
    if (Routine_State == ROUTINE_IDLE)
    {
        Routine_State = ROUTINE_RUNNING;
        Routine_StartTimer();
        return DCM_E_FORCE_RCRRP;  // 强制返回0x78
    }
    else if (Routine_State == ROUTINE_COMPLETED)
    {
        // 返回结果
        response[0] = Routine_Result;
        Routine_State = ROUTINE_IDLE;
        return E_OK;
    }
    return E_NOT_OK;
}
`

## DID范围定义

| 范围 | 用途 |
|------|------|
| 0xF180-0xF1FF | 车辆识别信息 (VIN等) |
| 0xF190-0xF1FF | 系统标识信息 |
| 0x0100-0xEFFF | 用户自定义DID |
| 0xF400-0xF8FF | OBD相关DID |

## 配置示例

### DCM General配置
`xml
<DcmGeneral>
    <DcmDevErrorDetect>true</DcmDevErrorDetect>
    <DcmRespondAllRequest>true</DcmRespondAllRequest>
    <DcmVersionInfoApi>true</DcmVersionInfoApi>
</DcmGeneral>
`

### 诊断服务配置
`xml
<DcmDsdService>
    <DcmDsdSidTabServiceId>0x22</DcmDsdSidTabServiceId>
    <DcmDsdSidTabSubfuncAvail>false</DcmDsdSidTabSubfuncAvail>
    <DcmDsdSidTabSecLevelRef>SecurityLevel_1</DcmDsdSidTabSecLevelRef>
    <DcmDsdSidTabSessionLevelRef>Session_Default</DcmDsdSidTabSessionLevelRef>
</DcmDsdService>
`

## 诊断会话控制

### 0x10 DiagnosticSessionControl

#### 会话类型
- 0x01: Default Session
- 0x02: Programming Session
- 0x03: Extended Diagnostic Session

#### 会话超时 (S3定时器)
- 默认会话超时: 5秒
- 扩展会话超时: 可配置 (通常5000ms)

## 安全访问 0x27

### Seed-Key机制
`
Tester:  27 01 (请求Seed)
ECU:     67 01 XX XX XX XX (返回Seed)
Tester:  27 02 YY YY YY YY (发送Key)
ECU:     67 02 (验证通过) 或 7F 27 35 (验证失败)
`

### 安全级别
- Level 1: 基本安全访问
- Level 2: 编程安全访问

## 故障码管理

### DTC状态字节
| Bit | 描述 |
|-----|------|
| 0   | TestFailed |
| 1   | TestFailedThisOperationCycle |
| 2   | PendingDTC |
| 3   | ConfirmedDTC |
| 4   | TestNotCompletedSinceLastClear |
| 5   | TestFailedSinceLastClear |
| 6   | TestNotCompletedThisOperationCycle |
| 7   | WarningIndicatorRequested |

### 0x19 ReadDTCInformation
- 0x02: 通过状态掩码读取DTC
- 0x04: 读取DTC快照记录
- 0x06: 读取DTC扩展数据
