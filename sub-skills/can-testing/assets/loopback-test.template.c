/**
 * CAN Loopback Test Template
 * 
 * Tests CAN TX/RX functionality in loopback mode.
 * No external hardware required.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* ============================================================================
 * Test Configuration
 * ============================================================================ */

#define TEST_ITERATIONS     100
#define TEST_TIMEOUT_MS     1000

/* ============================================================================
 * Test Statistics
 * ============================================================================ */

typedef struct {
    uint32_t tx_count;
    uint32_t rx_count;
    uint32_t error_count;
    uint32_t data_mismatch;
    uint32_t timeout_count;
} TestStats_t;

static TestStats_t test_stats = {0};

/* ============================================================================
 * Test Functions
 * ============================================================================ */

/**
 * @brief Initialize test
 */
void Test_Init(void)
{
    memset(&test_stats, 0, sizeof(test_stats));
    
    /* Enable loopback mode */
    CAN->BTR |= CAN_BTR_LBKM;
    
    /* Re-initialize CAN */
    CAN_Init();
}

/**
 * @brief Run loopback test
 * @return true if all tests passed
 */
bool Test_Loopback(void)
{
    CAN_TxMsg_t tx_msg;
    CAN_RxMsg_t rx_msg;
    
    Test_Init();
    
    for (uint32_t i = 0; i < TEST_ITERATIONS; i++) {
        /* Prepare test message */
        tx_msg.id = 0x123;
        tx_msg.ide = 0;
        tx_msg.rtr = 0;
        tx_msg.dlc = 8;
        
        /* Fill with test pattern */
        for (int j = 0; j < 8; j++) {
            tx_msg.data[j] = (uint8_t)(i + j);
        }
        
        /* Transmit */
        if (!CAN_TransmitBlocking(&tx_msg, TEST_TIMEOUT_MS)) {
            test_stats.timeout_count++;
            continue;
        }
        
        test_stats.tx_count++;
        
        /* Receive (should be immediate in loopback) */
        if (!CAN_Receive(&rx_msg)) {
            test_stats.error_count++;
            continue;
        }
        
        test_stats.rx_count++;
        
        /* Verify data */
        if (rx_msg.id != tx_msg.id ||
            rx_msg.dlc != tx_msg.dlc ||
            memcmp(rx_msg.data, tx_msg.data, tx_msg.dlc) != 0) {
            test_stats.data_mismatch++;
        }
    }
    
    /* Print results */
    Test_PrintResults();
    
    /* Check for pass */
    return (test_stats.rx_count == TEST_ITERATIONS &&
            test_stats.data_mismatch == 0);
}

/**
 * @brief Test different data lengths
 */
bool Test_DataLengths(void)
{
    CAN_TxMsg_t tx_msg;
    CAN_RxMsg_t rx_msg;
    bool all_passed = true;
    
    Test_Init();
    
    /* Test each DLC value */
    for (uint8_t dlc = 0; dlc <= 8; dlc++) {
        tx_msg.id = 0x100 + dlc;
        tx_msg.ide = 0;
        tx_msg.rtr = 0;
        tx_msg.dlc = dlc;
        
        /* Fill data */
        for (int j = 0; j < 8; j++) {
            tx_msg.data[j] = (uint8_t)dlc;
        }
        
        if (!CAN_TransmitBlocking(&tx_msg, TEST_TIMEOUT_MS)) {
            all_passed = false;
            continue;
        }
        
        if (!CAN_Receive(&rx_msg)) {
            all_passed = false;
            continue;
        }
        
        if (rx_msg.dlc != dlc) {
            all_passed = false;
        }
    }
    
    return all_passed;
}

/**
 * @brief Test different IDs
 */
bool Test_Identifiers(void)
{
    CAN_TxMsg_t tx_msg;
    CAN_RxMsg_t rx_msg;
    uint32_t test_ids[] = {0x000, 0x001, 0x7FF, 0x100, 0x555};
    bool all_passed = true;
    
    Test_Init();
    
    for (uint32_t i = 0; i < sizeof(test_ids)/sizeof(test_ids[0]); i++) {
        tx_msg.id = test_ids[i];
        tx_msg.ide = 0;
        tx_msg.rtr = 0;
        tx_msg.dlc = 8;
        memset(tx_msg.data, 0xAA, 8);
        
        if (!CAN_TransmitBlocking(&tx_msg, TEST_TIMEOUT_MS)) {
            all_passed = false;
            continue;
        }
        
        if (!CAN_Receive(&rx_msg)) {
            all_passed = false;
            continue;
        }
        
        if (rx_msg.id != test_ids[i]) {
            all_passed = false;
        }
    }
    
    return all_passed;
}

/**
 * @brief Test remote frames
 */
bool Test_RemoteFrames(void)
{
    CAN_TxMsg_t tx_msg;
    CAN_RxMsg_t rx_msg;
    bool all_passed = true;
    
    Test_Init();
    
    /* Send remote frame */
    tx_msg.id = 0x200;
    tx_msg.ide = 0;
    tx_msg.rtr = 1;
    tx_msg.dlc = 8;
    
    if (!CAN_TransmitBlocking(&tx_msg, TEST_TIMEOUT_MS)) {
        all_passed = false;
    }
    
    if (!CAN_Receive(&rx_msg)) {
        all_passed = false;
    } else if (rx_msg.rtr != 1) {
        all_passed = false;
    }
    
    return all_passed;
}

/**
 * @brief Print test results
 */
void Test_PrintResults(void)
{
    /* TODO: Implement print function */
    /*
    printf("=== CAN Loopback Test Results ===\n");
    printf("TX Count:     %lu\n", test_stats.tx_count);
    printf("RX Count:     %lu\n", test_stats.rx_count);
    printf("Errors:       %lu\n", test_stats.error_count);
    printf("Data Mismatch:%lu\n", test_stats.data_mismatch);
    printf("Timeouts:     %lu\n", test_stats.timeout_count);
    printf("Result:       %s\n", 
           (test_stats.rx_count == TEST_ITERATIONS) ? "PASS" : "FAIL");
    */
}

/**
 * @brief Run all tests
 */
void Test_RunAll(void)
{
    bool result;
    
    /* Test 1: Basic loopback */
    result = Test_Loopback();
    
    /* Test 2: Data lengths */
    result = Test_DataLengths() && result;
    
    /* Test 3: Identifiers */
    result = Test_Identifiers() && result;
    
    /* Test 4: Remote frames */
    result = Test_RemoteFrames() && result;
    
    /* Disable loopback mode */
    CAN->BTR &= ~CAN_BTR_LBKM;
}
