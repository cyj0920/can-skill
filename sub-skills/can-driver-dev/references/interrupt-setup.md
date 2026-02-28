# CAN Interrupt Setup

## Overview

CAN interrupts enable event-driven communication, reducing CPU polling overhead.

## Interrupt Types

| Interrupt | Trigger | Use Case |
|-----------|---------|----------|
| TX Complete | Mailbox empty | Send next message |
| RX Pending | Message in FIFO | Process received data |
| Error | Error condition | Fault handling |
| Wakeup | Bus activity | Low power mode |

## Enable Sequence

### Step 1: Configure NVIC

```c
// Enable CAN TX interrupt
NVIC_SetPriority(CAN1_TX_IRQn, 1);
NVIC_EnableIRQ(CAN1_TX_IRQn);

// Enable CAN RX0 interrupt
NVIC_SetPriority(CAN1_RX0_IRQn, 2);
NVIC_EnableIRQ(CAN1_RX0_IRQn);

// Enable CAN error interrupt
NVIC_SetPriority(CAN1_SCE_IRQn, 3);
NVIC_EnableIRQ(CAN1_SCE_IRQn);
```

### Step 2: Enable CAN Interrupts

```c
// TX mailbox empty interrupt
CAN->IER |= CAN_IER_TMEIE;

// RX FIFO 0 message pending interrupt
CAN->IER |= CAN_IER_FMPIE0;

// Error interrupts
CAN->IER |= CAN_IER_ERRIE | CAN_IER_BOFIE | CAN_IER_EPVIE;
```

## TX Interrupt

### Enable
```c
CAN->IER |= CAN_IER_TMEIE;
```

### ISR
```c
void CAN1_TX_IRQHandler(void)
{
    if (CAN->TSR & CAN_TSR_RQCP0) {
        // TX mailbox 0 complete
        CAN->TSR |= CAN_TSR_RQCP0;  // Clear flag
        
        // Send next message if pending
        CAN_SendNext();
    }
    // Check other mailboxes...
}
```

## RX Interrupt

### Enable
```c
CAN->IER |= CAN_IER_FMPIE0;  // FIFO 0
// or
CAN->IER |= CAN_IER_FMPIE1;  // FIFO 1
```

### ISR
```c
void CAN1_RX0_IRQHandler(void)
{
    if (CAN->RF0R & CAN_RF0R_FMP0) {
        // Read message
        uint32_t id = CAN->sFIFOMailBox[0].RIR >> 21;
        uint32_t dlc = CAN->sFIFOMailBox[0].RDTR & 0xF;
        
        uint8_t data[8];
        uint32_t *p = (uint32_t*)data;
        p[0] = CAN->sFIFOMailBox[0].RDLR;
        p[1] = CAN->sFIFOMailBox[0].RDHR;
        
        // Release FIFO
        CAN->RF0R |= CAN_RF0R_RFOM0;
        
        // Process message
        CAN_ProcessMessage(id, data, dlc);
    }
}
```

## Error Interrupt

### Enable
```c
CAN->IER |= CAN_IER_ERRIE;    // Error interrupt
CAN->IER |= CAN_IER_BOFIE;    // Bus-off
CAN->IER |= CAN_IER_EPVIE;    // Error passive
CAN->IER |= CAN_IER_EWGIE;    // Warning
CAN->IER |= CAN_IER_LECIE;    // Last error code
```

### ISR
```c
void CAN1_SCE_IRQHandler(void)
{
    uint32_t esr = CAN->ESR;
    
    if (CAN->MSR & CAN_MSR_ERRI) {
        // Clear error interrupt flag
        CAN->MSR &= ~CAN_MSR_ERRI;
        
        // Check bus-off
        if (esr & CAN_ESR_BOFF) {
            // Handle bus-off
            CAN->MCR |= CAN_MCR_ABOM;  // Auto recovery
        }
        
        // Check error passive
        if (esr & CAN_ESR_EPVF) {
            uint8_t tec = (esr >> 16) & 0xFF;
            uint8_t rec = (esr >> 24) & 0xFF;
            // Handle error passive
        }
        
        // Check last error code
        uint8_t lec = (esr >> 4) & 0x7;
        if (lec) {
            // Handle specific error
        }
    }
}
```

## Priority Recommendations

| Interrupt | Priority | Reason |
|-----------|----------|--------|
| RX | Higher | Avoid message loss |
| TX | Medium | Non-critical timing |
| Error | Lower | Diagnostic purpose |

```c
// Priority example (lower number = higher priority)
NVIC_SetPriority(CAN1_RX0_IRQn, 1);  // Highest
NVIC_SetPriority(CAN1_TX_IRQn, 2);
NVIC_SetPriority(CAN1_SCE_IRQn, 3);  // Lowest
```

## Interrupt Flags Clear

| Flag | Clear Method |
|------|--------------|
| RQCPx | Write 1 to TSR |
| FMP0 | Automatic after release |
| FOVR0 | Write 1 to RF0R |
| FULL0 | Write 1 to RF0R |
| ERRI | Write 0 to MSR |
| BOFF, EPVF, EWGF | Read ESR |

## Complete Setup Example

```c
void CAN_Interrupt_Init(void)
{
    // 1. Disable all CAN interrupts
    CAN->IER = 0;
    
    // 2. Configure NVIC
    NVIC_SetPriority(CAN1_RX0_IRQn, 1);
    NVIC_SetPriority(CAN1_TX_IRQn, 2);
    NVIC_SetPriority(CAN1_SCE_IRQn, 3);
    
    NVIC_EnableIRQ(CAN1_RX0_IRQn);
    NVIC_EnableIRQ(CAN1_TX_IRQn);
    NVIC_EnableIRQ(CAN1_SCE_IRQn);
    
    // 3. Enable CAN interrupts
    CAN->IER = CAN_IER_FMPIE0 |   // RX FIFO 0 pending
               CAN_IER_TMEIE |    // TX mailbox empty
               CAN_IER_ERRIE |    // Error interrupt
               CAN_IER_BOFIE |    // Bus-off
               CAN_IER_EPVIE;     // Error passive
}
```
