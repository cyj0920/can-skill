# CAN MCAL 基础配置

## MCAL架构概述

MCAL (Microcontroller Abstraction Layer) 是AUTOSAR架构中直接操作硬件寄存器的底层驱动。

### MCAL驱动分组
1. **微控制器驱动组**: MCU、WDG、GPT等
2. **存储器驱动组**: Flash、EEPROM等
3. **通信驱动组**: CAN、SPI、LIN、FlexRay等
4. **输入输出驱动组**: Port、DIO、PWM、ADC等

## CAN MCAL配置流程

### 1. 基础依赖配置

#### MCU模块配置
- 系统时钟配置
- CAN控制器时钟使能
- 外设时钟分频

#### Port模块配置
- CAN引脚功能选择
- 输入/输出模式配置
- 上下拉电阻配置

### 2. CAN控制器寄存器配置

#### 主要寄存器
`c
// CAN控制寄存器
CAN->CR    // Control Register
CAN->BTR   // Bit Timing Register
CAN->IER   // Interrupt Enable Register
CAN->MSR   // Master Status Register
CAN->TSR   // Transmit Status Register
CAN->RF0R  // Receive FIFO 0 Register
CAN->RF1R  // Receive FIFO 1 Register
`

#### 位时序寄存器配置 (BTR)
`c
// 位时序计算公式
// BaudRate = CAN_Clock / ((BRP+1) * (SJW + BS1 + BS2))

// 示例: 500kbps @ 36MHz APB Clock
// Prescaler = 9, BS1 = 6, BS2 = 1
// BaudRate = 36MHz / (9 * (1+6+1)) = 500kbps
CAN->BTR = (9-1) |           // BRP
           ((6-1) << 16) |   // BS1
           ((1-1) << 20) |   // BS2
           ((1-1) << 24);    // SJW
`

### 3. 初始化流程

`c
void Can_Init(const Can_ConfigType* Config)
{
    // 1. 使能CAN时钟
    RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;
    
    // 2. 进入初始化模式
    CAN->MCR |= CAN_MCR_INRQ;
    while (!(CAN->MSR & CAN_MSR_INAK));
    
    // 3. 配置位时序
    CAN->BTR = Config->BaudrateConfig;
    
    // 4. 配置过滤器
    Can_ConfigFilter(&Config->FilterConfig);
    
    // 5. 退出初始化模式，进入正常模式
    CAN->MCR &= ~CAN_MCR_INRQ;
    while (CAN->MSR & CAN_MSR_INAK);
}
`

### 4. 中断配置

#### 中断类型
- **发送中断**: 邮箱发送完成
- **接收中断**: FIFO接收完成
- **错误中断**: 错误状态变化
- **BusOff中断**: 总线关闭

#### 中断优先级配置
`c
// NVIC配置
NVIC_SetPriority(CAN1_TX_IRQn, 5);
NVIC_SetPriority(CAN1_RX0_IRQn, 5);
NVIC_SetPriority(CAN1_RX1_IRQn, 5);
NVIC_SetPriority(CAN1_SCE_IRQn, 5);

NVIC_EnableIRQ(CAN1_TX_IRQn);
NVIC_EnableIRQ(CAN1_RX0_IRQn);
NVIC_EnableIRQ(CAN1_RX1_IRQn);
NVIC_EnableIRQ(CAN1_SCE_IRQn);
`

### 5. 邮箱配置

#### 发送邮箱 (Tx Mailbox)
- 邮箱数量: 通常3个
- 配置: 标识符、数据长度、数据内容

#### 接收邮箱/FIFO (Rx FIFO)
- FIFO 0: 通常用于普通报文
- FIFO 1: 通常用于高优先级报文
- 深度: 通常3级FIFO

### 6. 过滤器配置

`c
void Can_ConfigFilter(Can_FilterTypeDef* FilterConfig)
{
    // 进入过滤器初始化模式
    CAN->FMR |= CAN_FMR_FINIT;
    
    // 配置过滤器模式 (ID Mask / ID List)
    CAN->FM1R |= FilterConfig->FilterMode;
    
    // 配置过滤器位宽 (16位 / 32位)
    CAN->FS1R |= FilterConfig->FilterScale;
    
    // 配置过滤器ID和Mask
    CAN->FilterRegister[FilterConfig->FilterBank].FR1 = FilterConfig->FilterId;
    CAN->FilterRegister[FilterConfig->FilterBank].FR2 = FilterConfig->FilterMask;
    
    // 激活过滤器
    CAN->FA1R |= (1 << FilterConfig->FilterBank);
    
    // 退出过滤器初始化模式
    CAN->FMR &= ~CAN_FMR_FINIT;
}
`

## 常见配置错误

### 1. 时钟未使能
`c
// 错误: 忘记使能CAN时钟
// 正确: RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;
`

### 2. 引脚未配置
`c
// 错误: 未配置CAN引脚为复用功能
// 正确: GPIO_Init(GPIOB, &GPIO_InitStruct);
`

### 3. 过滤器配置错误
`c
// 错误: 过滤器未激活
// 正确: CAN->FA1R |= (1 << FilterBank);
`

### 4. 中断未使能
`c
// 错误: NVIC使能但未使能CAN中断
// 正确: CAN->IER |= CAN_IER_TMEIE | CAN_IER_FMPIE0;
`

## 调试技巧

### 1. 寄存器检查
`c
// 读取关键寄存器状态
uint32_t can_cr = CAN->CR;
uint32_t can_msr = CAN->MSR;
uint32_t can_tsr = CAN->TSR;
uint32_t can_esr = CAN->ESR;
`

### 2. 错误计数器监控
`c
// 获取TEC和REC
uint8_t tec = (CAN->ESR >> 16) & 0xFF;
uint8_t rec = (CAN->ESR >> 24) & 0xFF;
`

### 3. BusOff检测
`c
if (CAN->ESR & CAN_ESR_BOFF)
{
    // BusOff处理
    Can_BusOffHandler();
}
`
