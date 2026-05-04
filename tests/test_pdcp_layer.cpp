#include <gtest/gtest.h>
#include "pdcp_layer.hpp"
#include <thread>

using namespace ue_simulator;

class PDCPLayerTest : public ::testing::Test {
protected:
    PDCPLayer pdcp;
};

// Test: PDCP header addition
TEST_F(PDCPLayerTest, AddHeader_ValidData_AddsHeaderAndTrailer) {
    std::vector<uint8_t> sdu = {0x01, 0x02, 0x03, 0x04};
    
    auto pdu = pdcp.addHeader(sdu);
    
    ASSERT_EQ(pdu.size(), sdu.size() + 2);
    EXPECT_EQ(pdu.front(), 0xAA);
    EXPECT_EQ(pdu.back(), 0xFF);
    EXPECT_EQ(pdu[1], 0x01);
    EXPECT_EQ(pdu[4], 0x04);
}

// Test: PDCP header addition with empty data
TEST_F(PDCPLayerTest, AddHeader_EmptyData_AddsOnlyHeaderAndTrailer) {
    std::vector<uint8_t> sdu;
    
    auto pdu = pdcp.addHeader(sdu);
    
    ASSERT_EQ(pdu.size(), 2);
    EXPECT_EQ(pdu[0], 0xAA);
    EXPECT_EQ(pdu[1], 0xFF);
}

// Test: PDCP header addition with large data
TEST_F(PDCPLayerTest, AddHeader_LargeData_HandlesCorrectly) {
    std::vector<uint8_t> sdu(1500, 0xAB);
    
    auto pdu = pdcp.addHeader(sdu);
    
    ASSERT_EQ(pdu.size(), 1502);
    EXPECT_EQ(pdu.front(), 0xAA);
    EXPECT_EQ(pdu.back(), 0xFF);
}

// Test: PDCP header removal
TEST_F(PDCPLayerTest, RemoveHeader_ValidPDU_RemovesHeaderAndTrailer) {
    std::vector<uint8_t> pdu = {0xAA, 0x01, 0x02, 0x03, 0xFF};
    
    auto sdu = pdcp.removeHeader(pdu);
    
    ASSERT_EQ(sdu.size(), 3);
    EXPECT_EQ(sdu[0], 0x01);
    EXPECT_EQ(sdu[1], 0x02);
    EXPECT_EQ(sdu[2], 0x03);
}

// Test: PDCP header removal with invalid PDU
TEST_F(PDCPLayerTest, RemoveHeader_TooSmallPDU_ReturnsEmpty) {
    std::vector<uint8_t> pdu = {0xAA};
    
    auto sdu = pdcp.removeHeader(pdu);
    
    EXPECT_TRUE(sdu.empty());
}

// Test: PDCP header removal with empty PDU
TEST_F(PDCPLayerTest, RemoveHeader_EmptyPDU_ReturnsEmpty) {
    std::vector<uint8_t> pdu;
    
    auto sdu = pdcp.removeHeader(pdu);
    
    EXPECT_TRUE(sdu.empty());
}

// Test: Round-trip (add then remove header)
TEST_F(PDCPLayerTest, RoundTrip_AddThenRemove_ReturnsOriginalData) {
    std::vector<uint8_t> original = {0x10, 0x20, 0x30, 0x40, 0x50};
    
    auto pdu = pdcp.addHeader(original);
    auto recovered = pdcp.removeHeader(pdu);
    
    EXPECT_EQ(original, recovered);
}

// Test: Statistics after operations
TEST_F(PDCPLayerTest, GetStatistics_AfterOperations_ReturnsCorrectCounts) {
    PDU pdu1;
    pdu1.data = {0x01, 0x02, 0x03};
    pdcp.sendPDU(pdu1);
    
    PDU pdu2;
    pdu2.data = {0x04, 0x05};
    pdcp.sendPDU(pdu2);
    
    auto stats = pdcp.getStatistics();
    
    EXPECT_EQ(stats.transmittedPackets, 2);
    EXPECT_EQ(stats.transmittedBytes, 5);
}

// Test: Thread safety - concurrent header additions
TEST_F(PDCPLayerTest, AddHeader_ConcurrentCalls_ThreadSafe) {
    const int numThreads = 10;
    const int operationsPerThread = 100;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, operationsPerThread]() {
            for (int j = 0; j < operationsPerThread; ++j) {
                std::vector<uint8_t> data = {0x01, 0x02, 0x03};
                auto pdu = pdcp.addHeader(data);
                EXPECT_EQ(pdu.size(), 5);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
}

// Test: Data integrity with special bytes
TEST_F(PDCPLayerTest, AddHeader_DataContainsHeaderBytes_PreservesData) {
    std::vector<uint8_t> sdu = {0xAA, 0xFF, 0xAA, 0xFF};
    
    auto pdu = pdcp.addHeader(sdu);
    auto recovered = pdcp.removeHeader(pdu);
    
    EXPECT_EQ(sdu, recovered);
}
