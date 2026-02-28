/**
 * CAN Filter Configuration Template
 * 
 * This template provides CAN filter configuration examples.
 * Adapt register names and addresses for your specific MCU.
 */

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * Filter Configuration Examples
 * ============================================================================ */

/**
 * @brief Configure filter to accept all messages
 */
void CAN_Filter_AcceptAll(void)
{
    /* Enter filter initialization mode */
    CAN->FMR |= (1U << 0);
    
    /* Deactivate filter 0 */
    CAN->FA1R &= ~(1U << 0);
    
    /* Set 32-bit scale */
    CAN->FS1R |= (1U << 0);
    
    /* Set mask mode */
    CAN->FM1R &= ~(1U << 0);
    
    /* Set ID and mask (both 0 = accept all) */
    CAN->sFilterRegister[0].FR1 = 0;
    CAN->sFilterRegister[0].FR2 = 0;
    
    /* Assign to FIFO 0 */
    CAN->FFA1R &= ~(1U << 0);
    
    /* Activate filter 0 */
    CAN->FA1R |= (1U << 0);
    
    /* Exit filter initialization mode */
    CAN->FMR &= ~(1U << 0);
}

/**
 * @brief Configure filter for single standard ID
 * @param id Standard ID to accept (11-bit)
 */
void CAN_Filter_SingleStdId(uint16_t id)
{
    /* Enter filter initialization mode */
    CAN->FMR |= (1U << 0);
    
    /* Deactivate filter 0 */
    CAN->FA1R &= ~(1U << 0);
    
    /* Set 32-bit scale */
    CAN->FS1R |= (1U << 0);
    
    /* Set mask mode */
    CAN->FM1R &= ~(1U << 0);
    
    /* Set ID (shifted for standard ID position) */
    CAN->sFilterRegister[0].FR1 = (id << 21);
    
    /* Set mask - all ID bits must match */
    CAN->sFilterRegister[0].FR2 = (0x7FF << 21);
    
    /* Assign to FIFO 0 */
    CAN->FFA1R &= ~(1U << 0);
    
    /* Activate filter 0 */
    CAN->FA1R |= (1U << 0);
    
    /* Exit filter initialization mode */
    CAN->FMR &= ~(1U << 0);
}

/**
 * @brief Configure filter for ID range
 * @param base_id Base ID of range
 * @param mask Mask for range (set bits must match)
 */
void CAN_Filter_IdRange(uint16_t base_id, uint16_t mask)
{
    /* Enter filter initialization mode */
    CAN->FMR |= (1U << 0);
    
    /* Deactivate filter 0 */
    CAN->FA1R &= ~(1U << 0);
    
    /* Set 32-bit scale */
    CAN->FS1R |= (1U << 0);
    
    /* Set mask mode */
    CAN->FM1R &= ~(1U << 0);
    
    /* Set base ID */
    CAN->sFilterRegister[0].FR1 = (base_id << 21);
    
    /* Set mask */
    CAN->sFilterRegister[0].FR2 = (mask << 21);
    
    /* Assign to FIFO 0 */
    CAN->FFA1R &= ~(1U << 0);
    
    /* Activate filter 0 */
    CAN->FA1R |= (1U << 0);
    
    /* Exit filter initialization mode */
    CAN->FMR &= ~(1U << 0);
}

/**
 * @brief Configure filter for multiple IDs (list mode)
 * @param id1 First ID to accept
 * @param id2 Second ID to accept
 */
void CAN_Filter_TwoIds(uint16_t id1, uint16_t id2)
{
    /* Enter filter initialization mode */
    CAN->FMR |= (1U << 0);
    
    /* Deactivate filter 0 */
    CAN->FA1R &= ~(1U << 0);
    
    /* Set 32-bit scale */
    CAN->FS1R |= (1U << 0);
    
    /* Set list mode (two IDs in FR1 and FR2) */
    CAN->FM1R |= (1U << 0);
    
    /* Set two IDs */
    CAN->sFilterRegister[0].FR1 = (id1 << 21);
    CAN->sFilterRegister[0].FR2 = (id2 << 21);
    
    /* Assign to FIFO 0 */
    CAN->FFA1R &= ~(1U << 0);
    
    /* Activate filter 0 */
    CAN->FA1R |= (1U << 0);
    
    /* Exit filter initialization mode */
    CAN->FMR &= ~(1U << 0);
}

/**
 * @brief Configure filter for four IDs (16-bit list mode)
 * @param ids Array of 4 IDs
 */
void CAN_Filter_FourIds(const uint16_t ids[4])
{
    /* Enter filter initialization mode */
    CAN->FMR |= (1U << 0);
    
    /* Deactivate filter 0 */
    CAN->FA1R &= ~(1U << 0);
    
    /* Set 16-bit scale */
    CAN->FS1R &= ~(1U << 0);
    
    /* Set list mode */
    CAN->FM1R |= (1U << 0);
    
    /* Pack four IDs into two registers */
    /* FR1: ID1 (bits 31-21) + ID2 (bits 15-5) */
    CAN->sFilterRegister[0].FR1 = (ids[0] << 21) | (ids[1] << 5);
    
    /* FR2: ID3 (bits 31-21) + ID4 (bits 15-5) */
    CAN->sFilterRegister[0].FR2 = (ids[2] << 21) | (ids[3] << 5);
    
    /* Assign to FIFO 0 */
    CAN->FFA1R &= ~(1U << 0);
    
    /* Activate filter 0 */
    CAN->FA1R |= (1U << 0);
    
    /* Exit filter initialization mode */
    CAN->FMR &= ~(1U << 0);
}

/**
 * @brief Configure filter for extended ID
 * @param id Extended ID to accept (29-bit)
 */
void CAN_Filter_ExtendedId(uint32_t id)
{
    /* Enter filter initialization mode */
    CAN->FMR |= (1U << 0);
    
    /* Deactivate filter 0 */
    CAN->FA1R &= ~(1U << 0);
    
    /* Set 32-bit scale */
    CAN->FS1R |= (1U << 0);
    
    /* Set mask mode */
    CAN->FM1R &= ~(1U << 0);
    
    /* Set extended ID with IDE bit set */
    CAN->sFilterRegister[0].FR1 = (id << 3) | (1U << 2);
    
    /* Set mask - all ID bits must match, plus IDE */
    CAN->sFilterRegister[0].FR2 = (0x1FFFFFFF << 3) | (1U << 2);
    
    /* Assign to FIFO 0 */
    CAN->FFA1R &= ~(1U << 0);
    
    /* Activate filter 0 */
    CAN->FA1R |= (1U << 0);
    
    /* Exit filter initialization mode */
    CAN->FMR &= ~(1U << 0);
}

/* ============================================================================
 * Multiple Filter Configuration
 * ============================================================================ */

/**
 * @brief Configure multiple filters
 * Example: Filter 0 for IDs 0x100-0x1FF, Filter 1 for ID 0x200
 */
void CAN_Filter_MultipleExample(void)
{
    /* Enter filter initialization mode */
    CAN->FMR |= (1U << 0);
    
    /* === Filter 0: ID range 0x100-0x1FF === */
    CAN->FA1R &= ~(1U << 0);        /* Deactivate */
    CAN->FS1R |= (1U << 0);         /* 32-bit */
    CAN->FM1R &= ~(1U << 0);        /* Mask mode */
    CAN->FFA1R &= ~(1U << 0);       /* FIFO 0 */
    
    CAN->sFilterRegister[0].FR1 = (0x100 << 21);  /* Base ID */
    CAN->sFilterRegister[0].FR2 = (0x700 << 21);  /* Mask: upper 4 bits */
    
    CAN->FA1R |= (1U << 0);         /* Activate */
    
    /* === Filter 1: Single ID 0x200 === */
    CAN->FA1R &= ~(1U << 1);        /* Deactivate */
    CAN->FS1R |= (1U << 1);         /* 32-bit */
    CAN->FM1R &= ~(1U << 1);        /* Mask mode */
    CAN->FFA1R |= (1U << 1);        /* FIFO 1 */
    
    CAN->sFilterRegister[1].FR1 = (0x200 << 21);
    CAN->sFilterRegister[1].FR2 = (0x7FF << 21);
    
    CAN->FA1R |= (1U << 1);         /* Activate */
    
    /* Exit filter initialization mode */
    CAN->FMR &= ~(1U << 0);
}
