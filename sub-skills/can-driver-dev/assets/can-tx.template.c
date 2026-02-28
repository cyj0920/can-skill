/**
 * CAN Transmit Template
 * 
 * This template provides CAN transmission functions.
 * Adapt register names and addresses for your specific MCU.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* ============================================================================
 * Configuration
 * ============================================================================ */

/* Timeout for TX operations */
#define CAN_TX_TIMEOUT      1000U   /* milliseconds */

/* ============================================================================
 * Type Definitions
 * ============================================================================ */

/**
 * @brief CAN message structure
 */
typedef struct {
    uint32_t id;            /* Standard or Extended ID */
    uint8_t  ide;           /* 0=Standard (11-bit), 1=Extended (29-bit) */
    uint8_t  rtr;           /* 0=Data frame, 1=Remote frame */
    uint8_t  dlc;           /* Data Length Code (0-8) */
    uint8_t  data[8];       /* Data payload */
} CAN_TxMsg_t;

/* ============================================================================
 * Register Definitions
 * ============================================================================ */

/* TX mailbox registers */
typedef struct {
    volatile uint32_t TIR;  /* TX Identifier Register */
    volatile uint32_t TDTR; /* TX Data Length Register */
    volatile uint32_t TDLR; /* TX Data Low Register */
    volatile uint32_t TDHR; /* TX Data High Register */
} CAN_TxMailBox_TypeDef;

/* Register bit definitions */
#define CAN_TSR_TME0        (1U << 26)   /* TX Mailbox 0 Empty */
#define CAN_TSR_TME1        (1U << 27)   /* TX Mailbox 1 Empty */
#define CAN_TSR_TME2        (1U << 28)   /* TX Mailbox 2 Empty */
#define CAN_TSR_RQCP0       (1U << 0)    /* Request Complete Mailbox 0 */
#define CAN_TIR_TXRQ        (1U << 0)    /* TX Request */
#define CAN_TIR_RTR         (1U << 1)    /* Remote TX Request */
#define CAN_TIR_IDE         (1U << 2)    /* ID Extended */

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/**
 * @brief Transmit a CAN message
 * @param msg Pointer to message structure
 * @return true if transmitted successfully
 */
bool CAN_Transmit(const CAN_TxMsg_t *msg);

/**
 * @brief Transmit with blocking wait
 * @param msg Pointer to message structure
 * @param timeout_ms Timeout in milliseconds
 * @return true if transmitted and acknowledged
 */
bool CAN_TransmitBlocking(const CAN_TxMsg_t *msg, uint32_t timeout_ms);

/**
 * @brief Check if TX mailbox is available
 * @return true if at least one mailbox is empty
 */
bool CAN_IsTxReady(void);

/**
 * @brief Get empty mailbox number
 * @return Mailbox number (0-2) or -1 if none available
 */
int8_t CAN_GetEmptyMailbox(void);

/* ============================================================================
 * Implementation
 * ============================================================================ */

bool CAN_Transmit(const CAN_TxMsg_t *msg)
{
    int8_t mailbox;
    CAN_TxMailBox_TypeDef *tx_mb;
    
    /* Validate input */
    if (msg == NULL || msg->dlc > 8) {
        return false;
    }
    
    /* Get empty mailbox */
    mailbox = CAN_GetEmptyMailbox();
    if (mailbox < 0) {
        return false;  /* No empty mailbox */
    }
    
    tx_mb = &CAN->sTxMailBox[mailbox];
    
    /* Configure identifier */
    if (msg->ide) {
        /* Extended ID (29-bit) */
        tx_mb->TIR = (msg->id << 3) | CAN_TIR_IDE;
    } else {
        /* Standard ID (11-bit) */
        tx_mb->TIR = (msg->id << 21);
    }
    
    /* Configure RTR if remote frame */
    if (msg->rtr) {
        tx_mb->TIR |= CAN_TIR_RTR;
    }
    
    /* Configure data length */
    tx_mb->TDTR = msg->dlc & 0x0F;
    
    /* Write data */
    tx_mb->TDLR = ((uint32_t)msg->data[3] << 24) |
                  ((uint32_t)msg->data[2] << 16) |
                  ((uint32_t)msg->data[1] << 8)  |
                  ((uint32_t)msg->data[0] << 0);
    
    tx_mb->TDHR = ((uint32_t)msg->data[7] << 24) |
                  ((uint32_t)msg->data[6] << 16) |
                  ((uint32_t)msg->data[5] << 8)  |
                  ((uint32_t)msg->data[4] << 0);
    
    /* Request transmission */
    tx_mb->TIR |= CAN_TIR_TXRQ;
    
    return true;
}

bool CAN_TransmitBlocking(const CAN_TxMsg_t *msg, uint32_t timeout_ms)
{
    uint32_t start_time = 0;  /* TODO: Get current time */
    uint32_t mailbox_mask;
    
    /* Transmit message */
    if (!CAN_Transmit(msg)) {
        return false;
    }
    
    /* Determine which mailbox was used */
    if (CAN->TSR & CAN_TSR_TME0) {
        mailbox_mask = CAN_TSR_RQCP0;
    } else if (CAN->TSR & CAN_TSR_TME1) {
        mailbox_mask = CAN_TSR_RQCP1;
    } else {
        mailbox_mask = CAN_TSR_RQCP2;
    }
    
    /* Wait for completion */
    while (!(CAN->TSR & mailbox_mask)) {
        /* TODO: Check timeout
         * if ((GetTick() - start_time) >= timeout_ms) {
         *     return false;
         * }
         */
    }
    
    /* Clear the request complete flag */
    CAN->TSR = mailbox_mask;
    
    return true;
}

bool CAN_IsTxReady(void)
{
    return (CAN->TSR & (CAN_TSR_TME0 | CAN_TSR_TME1 | CAN_TSR_TME2)) != 0;
}

int8_t CAN_GetEmptyMailbox(void)
{
    if (CAN->TSR & CAN_TSR_TME0) return 0;
    if (CAN->TSR & CAN_TSR_TME1) return 1;
    if (CAN->TSR & CAN_TSR_TME2) return 2;
    return -1;
}

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

/**
 * @brief Transmit standard ID message (convenience function)
 */
bool CAN_TransmitStd(uint32_t id, const uint8_t *data, uint8_t len)
{
    CAN_TxMsg_t msg = {
        .id = id,
        .ide = 0,
        .rtr = 0,
        .dlc = len > 8 ? 8 : len
    };
    
    if (data != NULL && len > 0) {
        memcpy(msg.data, data, msg.dlc);
    }
    
    return CAN_Transmit(&msg);
}

/**
 * @brief Transmit extended ID message (convenience function)
 */
bool CAN_TransmitExt(uint32_t id, const uint8_t *data, uint8_t len)
{
    CAN_TxMsg_t msg = {
        .id = id,
        .ide = 1,
        .rtr = 0,
        .dlc = len > 8 ? 8 : len
    };
    
    if (data != NULL && len > 0) {
        memcpy(msg.data, data, msg.dlc);
    }
    
    return CAN_Transmit(&msg);
}

/**
 * @brief Transmit remote frame (convenience function)
 */
bool CAN_TransmitRemote(uint32_t id, uint8_t dlc)
{
    CAN_TxMsg_t msg = {
        .id = id,
        .ide = 0,
        .rtr = 1,
        .dlc = dlc > 8 ? 8 : dlc,
        .data = {0}
    };
    
    return CAN_Transmit(&msg);
}
