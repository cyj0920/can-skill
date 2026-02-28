# CAN Register Bits Reference

## Control Register (MCR)

### STM32 bxCAN
| Bit | Name | Description |
|-----|------|-------------|
| 0 | INRQ | Initialization Request |
| 1 | SLEEP | Sleep Mode Request |
| 2 | TXFP | TX FIFO Priority |
| 3 | RFLM | RX FIFO Locked Mode |
| 4 | NART | No Automatic Retransmission |
| 6 | AWUM | Automatic Wakeup Mode |
| 7 | ABOM | Automatic Bus-Off Management |
| 15 | RESET | Software Reset |

### Key Operations
```c
// Enter init mode
CAN->MCR |= CAN_MCR_INRQ;
while(!(CAN->MSR & CAN_MSR_INAK));

// Exit init mode
CAN->MCR &= ~CAN_MCR_INRQ;
while(CAN->MSR & CAN_MSR_INAK);
```

## Status Register (MSR)

| Bit | Name | Description |
|-----|------|-------------|
| 0 | INAK | Initialization Acknowledge |
| 1 | SLAK | Sleep Acknowledge |
| 2 | ERRI | Error Interrupt |
| 3 | WKUI | Wakeup Interrupt Flag |
| 4 | SLAKI | Sleep Interrupt Flag |

## Bit Timing Register (BTR)

See `timing-config.md` for detailed configuration.

## TX Mailbox Registers

### TX Mailbox Identifier (TIxR)
| Bit | Name | Description |
|-----|------|-------------|
| 0-2 | TXRQ | Request TX, Abort TX |
| 1 | RTR | Remote TX Request |
| 2 | IDE | ID Extended |
| 3-31 | STID/EXID | Standard/Extended ID |

### TX Mailbox Data Length (TDTxR)
| Bit | Name | Description |
|-----|------|-------------|
| 0-3 | DLC | Data Length Code |
| 8 | TGT | Transmit Global Time |

### TX Mailbox Data (TDLxR, TDHxR)
- TDLxR: Data bytes 0-3
- TDHxR: Data bytes 4-7

## RX FIFO Registers

### RX FIFO Identifier (RIxR)
| Bit | Name | Description |
|-----|------|-------------|
| 0 | IDE | ID Extended |
| 1 | RTR | Remote TX Request |
| 2-31 | STID/EXID | Standard/Extended ID |

### RX FIFO Data Length (RDTxR)
| Bit | Name | Description |
|-----|------|-------------|
| 0-3 | DLC | Data Length Code |
| 8-15 | FMI | Filter Match Index |
| 16-31 | TIME | Time Stamp |

### RX FIFO Data (RDLxR, RDHxR)
- RDLxR: Data bytes 0-3
- RDHxR: Data bytes 4-7

## Filter Registers

### Filter Scale (FMR)
| Bit | Name | Description |
|-----|------|-------------|
| 0 | FINIT | Filter Init Mode |
| 8-13 | CAN2SB | CAN2 Start Bank |

### Filter Mode Register (FM1R)
- 0: Mask mode
- 1: List mode

### Filter Scale Register (FS1R)
- 0: Dual 16-bit
- 1: Single 32-bit

### Filter Activation Register (FA1R)
- Bit = 1: Filter enabled

### Filter Bank Registers (FxR1, FxR2)
- Mask mode: FxR1 = ID, FxR2 = Mask
- List mode: FxR1 = ID1, FxR2 = ID2

## Interrupt Enable Register (IER)

| Bit | Name | Description |
|-----|------|-------------|
| 0 | TMEIE | TX Mailbox Empty |
| 1 | FMPIE0 | FIFO 0 Message Pending |
| 2 | FFIE0 | FIFO 0 Full |
| 3 | FOVIE0 | FIFO 0 Overrun |
| 4 | FMPIE1 | FIFO 1 Message Pending |
| 5 | FFIE1 | FIFO 1 Full |
| 6 | FOVIE1 | FIFO 1 Overrun |
| 7 | WGIE | Warning Interrupt |
| 8 | EPVIE | Error Passive |
| 9 | BOFIE | Bus-Off |
| 10 | LECIE | Last Error Code |
| 11 | ERRIE | Error Interrupt |

## Error Register (ESR)

| Bit | Name | Description |
|-----|------|-------------|
| 0-7 | TEC | TX Error Counter |
| 8-15 | REC | RX Error Counter |
| 16-20 | LEC | Last Error Code |
| 21 | BOFF | Bus-Off Flag |
| 22 | EPVF | Error Passive Flag |
| 23 | EWGF | Error Warning Flag |

### Last Error Codes (LEC)
| Code | Error |
|------|-------|
| 0 | No Error |
| 1 | Stuff Error |
| 2 | Form Error |
| 3 | ACK Error |
| 4 | Bit Recessive Error |
| 5 | Bit Dominant Error |
| 6 | CRC Error |
| 7 | Controlled by software |
