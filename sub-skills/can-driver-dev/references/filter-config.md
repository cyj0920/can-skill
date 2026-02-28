# CAN Filter Configuration

## Overview

CAN acceptance filters reduce CPU load by filtering messages at hardware level.

## Filter Modes

### Mask Mode
- Uses ID + Mask pair
- Mask determines which ID bits are compared
- More flexible, handles ID ranges

### List Mode
- Uses two specific IDs
- Exact match only
- Simpler, less flexible

## Filter Scale

| Scale | Configuration |
|-------|---------------|
| 16-bit | Two 16-bit filters |
| 32-bit | One 32-bit filter |

## 32-bit Mask Mode

For standard ID (11-bit):

```
ID Register:
[31:21] STID[10:0] - Standard ID
[20:3]  Reserved
[2]     IDE - 0 for standard
[1]     RTR - 0 for data frame
[0]     Reserved

Mask Register:
[31:21] Mask[10:0] - 1 = compare, 0 = ignore
[20:3]  Reserved
[2]     IDE Mask
[1]     RTR Mask
[0]     Reserved
```

### Example: Accept ID 0x123

```c
// Filter ID
CAN->sFilterRegister[0].FR1 = (0x123 << 21);

// Mask: all bits must match
CAN->sFilterRegister[0].FR2 = (0x7FF << 21) | (1 << 1);

// Enable filter 0
CAN->FA1R |= 1;
```

### Example: Accept ID range 0x100-0x1FF

```c
// ID: 0x100
CAN->sFilterRegister[0].FR1 = (0x100 << 21);

// Mask: only upper 4 bits must match (0x1xx)
CAN->sFilterRegister[0].FR2 = (0x700 << 21);
```

## 16-bit Mask Mode

Two filters per bank:

```
FR1:
[31:21] ID1[10:0]   - First ID
[20:16] Reserved
[15:5]  Mask1[10:0] - First Mask
[4:0]   Reserved

FR2:
[31:21] ID2[10:0]   - Second ID
[20:16] Reserved
[15:5]  Mask2[10:0] - Second Mask
[4:0]   Reserved
```

### Example: Accept IDs 0x100, 0x200

```c
// Two exact IDs
CAN->sFilterRegister[0].FR1 = (0x100 << 21) | (0x7FF << 5);
CAN->sFilterRegister[0].FR2 = (0x200 << 21) | (0x7FF << 5);
```

## List Mode

### 32-bit List Mode

```c
// Two IDs: 0x123 and 0x456
CAN->FM1R |= (1 << 0);  // Set filter 0 to list mode
CAN->sFilterRegister[0].FR1 = (0x123 << 21);  // ID1
CAN->sFilterRegister[0].FR2 = (0x456 << 21);  // ID2
CAN->FA1R |= (1 << 0);  // Enable filter 0
```

### 16-bit List Mode

```c
// Four IDs: 0x100, 0x200, 0x300, 0x400
CAN->FM1R |= (1 << 0);  // List mode
CAN->FS1R &= ~(1 << 0); // 16-bit scale
CAN->sFilterRegister[0].FR1 = (0x100 << 21) | (0x200 << 5);
CAN->sFilterRegister[0].FR2 = (0x300 << 21) | (0x400 << 5);
CAN->FA1R |= (1 << 0);
```

## Extended ID (29-bit) Filtering

For extended ID, set IDE = 1:

```c
// Accept extended ID 0x12345678
uint32_t id = 0x12345678;
uint32_t mask = 0x1FFFFFFF;

CAN->sFilterRegister[0].FR1 = id << 3 | (1 << 2);  // ID + IDE=1
CAN->sFilterRegister[0].FR2 = mask << 3 | (1 << 2); // Mask + IDE=1
```

## FIFO Assignment

Assign filter to FIFO 0 or FIFO 1:

```c
// Filter 0 to FIFO 0 (default)
CAN->FFA1R &= ~(1 << 0);

// Filter 1 to FIFO 1
CAN->FFA1R |= (1 << 1);
```

## Filter Init Sequence

```c
void CAN_Filter_Init(void)
{
    // Enter filter init mode
    CAN->FMR |= CAN_FMR_FINIT;
    
    // Deactivate filter 0
    CAN->FA1R &= ~1;
    
    // Configure filter 0: 32-bit mask mode
    CAN->FM1R &= ~1;  // Mask mode
    CAN->FS1R |= 1;   // 32-bit scale
    CAN->FFA1R &= ~1; // FIFO 0
    
    // Set ID and mask
    CAN->sFilterRegister[0].FR1 = 0;
    CAN->sFilterRegister[0].FR2 = 0;  // Accept all
    
    // Activate filter 0
    CAN->FA1R |= 1;
    
    // Exit filter init mode
    CAN->FMR &= ~CAN_FMR_FINIT;
}
```

## Common Configurations

| Requirement | Mode | Scale | Example |
|-------------|------|-------|---------|
| Accept all | Mask | 32-bit | ID=0, Mask=0 |
| Single ID | Mask/List | 32-bit | ID=specific, Mask=ALL |
| ID range | Mask | 32-bit | ID=base, Mask=prefix |
| Multiple IDs | List | 16-bit | Up to 4 IDs per bank |
