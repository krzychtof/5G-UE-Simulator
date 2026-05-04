#include <gtest/gtest.h>
#include "rlc_layer.hpp"
#include <thread>
#include <chrono>

using namespace ue_simulator;

class RLCLayerTest : public ::testing::Test {
protected:
    RLCLayer rlc{RlcMode::UM};
};

// Test: Segment SDU creates PDU
TEST_F(RLCLayerTest, SegmentSDU_ValidData_CreatesPDU) {
    std::vector<uint8_t> sdu = {0x01, 0x02, 0x03};
    
    rlc.segmentSDU(sdu, Priority::HIGH);
    
    EXPECT_EQ(rlc.getQueueSize(), 1);
}

// Test: Multiple segmentations
TEST_F(RLCLayerTest, SegmentSDU_MultipleCalls_IncreasesQueueSize) {
    rlc.segmentSDU({0x01}, Priority::LOW);
    rlc.segmentSDU({0x02}, Priority::MEDIUM);
    rlc.segmentSDU({0x03}, Priority::HIGH);
    
    EXPECT_EQ(rlc.getQueueSize(), 3);
}

// Test: Receive PDU from queue
TEST_F(RLCLayerTest, ReceivePDU_AfterSegmentation_ReturnsPDU) {
    std::vector<uint8_t> sdu = {0x01, 0x02, 0x03};
    rlc.segmentSDU(sdu, Priority::HIGH);
    
    PDU pdu;
    bool result = rlc.receivePDU(pdu);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(pdu.data, sdu);
    EXPECT_EQ(pdu.priority, Priority::HIGH);
}

// Test: Receive from empty queue
TEST_F(RLCLayerTest, ReceivePDU_EmptyQueue_ReturnsFalse) {
    PDU pdu;
    bool result = rlc.receivePDU(pdu);
    
    EXPECT_FALSE(result);
}

// Test: FIFO order
TEST_F(RLCLayerTest, ReceivePDU_FIFO_ReturnsInOrder) {
    rlc.segmentSDU({0x01}, Priority::LOW);
    rlc.segmentSDU({0x02}, Priority::MEDIUM);
    rlc.segmentSDU({0x03}, Priority::HIGH);
    
    PDU pdu1, pdu2, pdu3;
    rlc.receivePDU(pdu1);
    rlc.receivePDU(pdu2);
    rlc.receivePDU(pdu3);
    
    EXPECT_EQ(pdu1.data[0], 0x01);
    EXPECT_EQ(pdu2.data[0], 0x02);
    EXPECT_EQ(pdu3.data[0], 0x03);
}

// Test: Queue size after operations
TEST_F(RLCLayerTest, GetQueueSize_AfterOperations_ReturnsCorrectSize) {
    rlc.segmentSDU({0x01}, Priority::LOW);
    rlc.segmentSDU({0x02}, Priority::LOW);
    EXPECT_EQ(rlc.getQueueSize(), 2);
    
    PDU pdu;
    rlc.receivePDU(pdu);
    EXPECT_EQ(rlc.getQueueSize(), 1);
    
    rlc.receivePDU(pdu);
    EXPECT_EQ(rlc.getQueueSize(), 0);
}

// Test: Is queue empty
TEST_F(RLCLayerTest, IsQueueEmpty_InitialState_ReturnsTrue) {
    EXPECT_TRUE(rlc.isQueueEmpty());
}

TEST_F(RLCLayerTest, IsQueueEmpty_AfterSegmentation_ReturnsFalse) {
    rlc.segmentSDU({0x01}, Priority::LOW);
    EXPECT_FALSE(rlc.isQueueEmpty());
}

// Test: PDU ID increments
TEST_F(RLCLayerTest, SegmentSDU_MultipleCalls_IncrementsID) {
    rlc.segmentSDU({0x01}, Priority::LOW);
    rlc.segmentSDU({0x02}, Priority::LOW);
    rlc.segmentSDU({0x03}, Priority::LOW);
    
    PDU pdu1, pdu2, pdu3;
    rlc.receivePDU(pdu1);
    rlc.receivePDU(pdu2);
    rlc.receivePDU(pdu3);
    
    EXPECT_EQ(pdu1.id, 0);
    EXPECT_EQ(pdu2.id, 1);
    EXPECT_EQ(pdu3.id, 2);
}

// Test: Priority preservation
TEST_F(RLCLayerTest, SegmentSDU_DifferentPriorities_PreservesPriority) {
    rlc.segmentSDU({0x01}, Priority::LOW);
    rlc.segmentSDU({0x02}, Priority::MEDIUM);
    rlc.segmentSDU({0x03}, Priority::HIGH);
    
    PDU pdu1, pdu2, pdu3;
    rlc.receivePDU(pdu1);
    rlc.receivePDU(pdu2);
    rlc.receivePDU(pdu3);
    
    EXPECT_EQ(pdu1.priority, Priority::LOW);
    EXPECT_EQ(pdu2.priority, Priority::MEDIUM);
    EXPECT_EQ(pdu3.priority, Priority::HIGH);
}

// Test: Timestamp is set
TEST_F(RLCLayerTest, SegmentSDU_CreatesPDU_SetsTimestamp) {
    auto before = std::chrono::steady_clock::now();
    rlc.segmentSDU({0x01}, Priority::LOW);
    auto after = std::chrono::steady_clock::now();
    
    PDU pdu;
    rlc.receivePDU(pdu);
    
    EXPECT_GE(pdu.timestamp, before);
    EXPECT_LE(pdu.timestamp, after);
}

// Test: Large data handling
TEST_F(RLCLayerTest, SegmentSDU_LargeData_HandlesCorrectly) {
    std::vector<uint8_t> largeData(10000, 0xAB);
    
    rlc.segmentSDU(largeData, Priority::HIGH);
    
    PDU pdu;
    rlc.receivePDU(pdu);
    
    EXPECT_EQ(pdu.data.size(), 10000);
    EXPECT_EQ(pdu.data[0], 0xAB);
}

// Test: Empty data handling
TEST_F(RLCLayerTest, SegmentSDU_EmptyData_CreatesEmptyPDU) {
    std::vector<uint8_t> emptyData;
    
    rlc.segmentSDU(emptyData, Priority::LOW);
    
    PDU pdu;
    rlc.receivePDU(pdu);
    
    EXPECT_TRUE(pdu.data.empty());
}

// Test: Thread safety - concurrent segmentations
TEST_F(RLCLayerTest, SegmentSDU_ConcurrentCalls_ThreadSafe) {
    const int numThreads = 10;
    const int operationsPerThread = 100;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, operationsPerThread]() {
            for (int j = 0; j < operationsPerThread; ++j) {
                rlc.segmentSDU({0x01, 0x02}, Priority::MEDIUM);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    EXPECT_EQ(rlc.getQueueSize(), numThreads * operationsPerThread);
}

// Test: Thread safety - concurrent receive
TEST_F(RLCLayerTest, ReceivePDU_ConcurrentCalls_ThreadSafe) {
    // Fill queue
    for (int i = 0; i < 1000; ++i) {
        rlc.segmentSDU({static_cast<uint8_t>(i)}, Priority::LOW);
    }
    
    std::atomic<int> receivedCount{0};
    const int numThreads = 10;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, &receivedCount]() {
            PDU pdu;
            while (rlc.receivePDU(pdu)) {
                receivedCount++;
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    EXPECT_EQ(receivedCount, 1000);
    EXPECT_TRUE(rlc.isQueueEmpty());
}

// Test: Statistics
TEST_F(RLCLayerTest, GetStatistics_AfterOperations_ReturnsCorrectStats) {
    rlc.segmentSDU({0x01, 0x02}, Priority::LOW);
    rlc.segmentSDU({0x03, 0x04, 0x05}, Priority::HIGH);
    
    auto stats = rlc.getStatistics();
    
    EXPECT_EQ(stats.queueSize, 2);
}

// Test: Queue overflow protection
TEST_F(RLCLayerTest, SegmentSDU_QueueFull_HandlesGracefully) {
    // Fill queue to max capacity
    for (size_t i = 0; i < 1000; ++i) {
        rlc.segmentSDU({0x01}, Priority::LOW);
    }
    
    // Try to add one more
    rlc.segmentSDU({0x02}, Priority::HIGH);
    
    // Should handle gracefully (implementation dependent)
    EXPECT_GE(rlc.getQueueSize(), 1000);
}

// Test: RLC mode
TEST_F(RLCLayerTest, Constructor_UMMode_CreatesCorrectly) {
    RLCLayer rlcUM(RlcMode::UM);
    EXPECT_TRUE(rlcUM.isQueueEmpty());
}

TEST_F(RLCLayerTest, Constructor_TMMode_CreatesCorrectly) {
    RLCLayer rlcTM(RlcMode::TM);
    EXPECT_TRUE(rlcTM.isQueueEmpty());
}

TEST_F(RLCLayerTest, Constructor_AMMode_CreatesCorrectly) {
    RLCLayer rlcAM(RlcMode::AM);
    EXPECT_TRUE(rlcAM.isQueueEmpty());
}
