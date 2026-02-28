/**
 * CAN Receive Template
 * 
 * This template provides CAN reception functions.
 * Adapt register names and addresses for your specific MCU.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* ============================================================================
 * Type Definitions
 * ============================================================================ */

/**
 * @brief CAN RX message structure
 */
typedef struct {
    uint32_t id;            /* Standard or Extended ID */
    uint8_t  ide;           /* 0=Standard (11-bit), 1=Extended (29-bit) */
    uint8_t  rtr;           /* 0=Data frame, 1=Remote frame */
    uint8_t  dlc;           /* Data Length Code (0-8) */
    uint8_t  data[8];       /* Data payload */
    uint8_t  fmi;           /* Filter Match Index */
    uint16_t timestamp;     /* Hardware timestamp (if available) */
} CAN_RxMsg_t;

/* ============================================================================
 * Register Definitions
 * ============================================================================ */

/* RX FIFO mailbox registers */
typedef struct {
    volatile uint32_t RIR;  /* RX Identifier Register */
    volatile uint32_t RDTR; /* RX Data Length Register */
    volatile uint32_t RDLR; /* RX Data Low Register */
    volatile uint32_t RDHR; /* RX Data High Register */
} CAN_RxFIFO_TypeDef;

/* Register bit definitions */
#define CAN_RF0R_FMP0        (0x03U << 0)  /* FIFO 0 Message Pending */
#define CAN_RF0R_FULL0       (1U << 3)     /* FIFO 0 Full */
#define CAN_RF0R_FOVR0       (1U << 4)     /* FIFO 0 Overrun */
#define CAN_RF0R_RFOM0       (1U << 5)     /* Release FIFO 0 Output */

#define CAN_RIR_IDE          (1U << 2)     /* ID Extended */
#define CAN_RIR_RTR          (1U << 1)     /* Remote TX Request */

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/**
 * @brief Check if RX message is pending
 * @return true if message available
 */
bool CAN_IsRxMessage(void);

/**
 * @brief Receive a CAN message (polling)
 * @param msg Pointer to message structure to fill
 * @return true if message received
 */
bool CAN_Receive(CAN_RxMsg_t *msg);

/**
 * @brief Get number of pending RX messages
 * @return Number of messages in FIFO
 */
uint8_t CAN_GetRxCount(void);

/* ============================================================================
 * Implementation - Polling Mode
 * ============================================================================ */

bool CAN_IsRxMessage(void)
{
    return (CAN->RF0R & CAN_RF0R_FMP0) != 0;
}

uint8_t CAN_GetRxCount(void)
{
    return CAN->RF0R & CAN_RF0R_FMP0;
}

bool CAN_Receive(CAN_RxMsg_t *msg)
{
    CAN_RxFIFO_TypeDef *rx_fifo;
    
    if (msg == NULL) {
        return false;
    }
    
    /* Check if message available */
    if (!CAN_IsRxMessage()) {
        return false;
    }
    
    /* Read from FIFO 0 */
    rx_fifo = &CAN->sFIFOMailBox[0];
    
    /* Extract ID and flags */
    uint32_t rir = rx_fifo->RIR;
    
    if (rir & CAN_RIR_IDE) {
        /* Extended ID (29-bit) */
        msg->ide = 1;
        msg->id = rir >> 3;
    } else {
        /* Standard ID (11-bit) */
        msg->ide = 0;
        msg->id = rir >> 21;
    }
    
    msg->rtr = (rir & CAN_RIR_RTR) ? 1 : 0;
    
    /* Extract DLC and metadata */
    uint32_t rdtr = rx_fifo->RDTR;
    msg->dlc = rdtr & 0x0F;
    msg->fmi = (rdtr >> 8) & 0xFF;
    msg->timestamp = (rdtr >> 16) & 0xFFFF;
    
    /* Extract data */
    uint32_t rdlr = rx_fifo->RDLR;
    uint32_t rdhr = rx_fifo->RDHR;
    
    msg->data[0] = (rdlr >> 0)  & 0xFF;
    msg->data[1] = (rdlr >> 8)  & 0xFF;
    msg->data[2] = (rdlr >> 16) & 0xFF;
    msg->data[3] = (rdlr >> 24) & 0xFF;
    msg->data[4] = (rdhr >> 0)  & 0xFF;
    msg->data[5] = (rdhr >> 8)  & 0xFF;
    msg->data[6] = (rdhr >> 16) & 0xFF;
    msg->data[7] = (rdhr >> 24) & 0xFF;
    
    /* Release FIFO */
    CAN->RF0R |= CAN_RF0R_RFOM0;
    
    return true;
}

/* ============================================================================
 * Implementation - Interrupt Mode
 * ============================================================================ */

/* RX callback function pointer */
typedef void (*CAN_RxCallback_t)(const CAN_RxMsg_t *msg);
static CAN_RxCallback_t rx_callback = NULL;

/**
 * @brief Register RX callback
 * @param callback Function to call on RX
 */
void CAN_RegisterRxCallback(CAN_RxCallback_t callback)
{
    rx_callback = callback;
}

/**
 * @brief RX Interrupt Handler
 * Call this from your ISR (e.g., CAN1_RX0_IRQHandler)
 */
void CAN_RX_IRQHandler(void)
{
    CAN_RxMsg_t msg;
    
    /* Check for overrun */
    if (CAN->RF0R & CAN_RF0R_FOVR0) {
        CAN->RF0R |= CAN_RF0R_FOVR0;  /* Clear overrun flag */
        /* Handle overrun if needed */
    }
    
    /* Process all pending messages */
    while (CAN_IsRxMessage()) {
        if (CAN_Receive(&msg)) {
            if (rx_callback != NULL) {
                rx_callback(&msg);
            }
        }
    }
}

/* ============================================================================
 * Interrupt Setup
 * ============================================================================ */

/**
 * @brief Enable RX interrupt
 */
void CAN_EnableRxInterrupt(void)
{
    /* Enable RX FIFO 0 message pending interrupt */
    CAN->IER |= CAN_IER_FMPIE0;
    
    /* Enable interrupt in NVIC */
    /* NVIC_EnableIRQ(CAN1_RX0_IRQn); */
}

/**
 * @brief Disable RX interrupt
 */
void CAN_DisableRxInterrupt(void)
{
    CAN->IER &= ~CAN_IER_FMPIE0;
}

/* ============================================================================
 * Example Usage
 * ============================================================================ */

/*
// Polling mode example:
void main(void)
{
    CAN_RxMsg_t msg;
    
    CAN_Init();
    
    while (1) {
        if (CAN_IsRxMessage()) {
            CAN_Receive(&msg);
            
            // Process received message
            if (msg.id == 0x123) {
                // Handle message with ID 0x123
            }
        }
    }
}

// Interrupt mode example:
void rx_callback(const CAN_RxMsg_t *msg)
{
    // Process message in interrupt context
    // Keep this function short!
    process_message(msg);
}

void main(void)
{
    CAN_Init();
    CAN_RegisterRxCallback(rx_callback);
    CAN_EnableRxInterrupt();
    
    while (1) {
        // Main loop can do other tasks
    }
}
*/
