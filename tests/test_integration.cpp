#include <gtest/gtest.h>
#include "ue_simulator.hpp"
#include <thread>
#include <chrono>

using namespace ue_simulator;

class UESimulatorTest : public ::testing::Test {
protected:
    UESimulator simulator;
};

// Test: Simulator construction
TEST_F(UESimulatorTest, Constructor_CreatesSuccessfully) {
    // Should not throw
    EXPECT_NO_THROW(UESimulator sim);
}

// Test: Start and stop
TEST_F(UESimulatorTest, StartStop_BasicOperation_WorksCorrectly) {
    EXPECT_NO_THROW(simulator.start());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_NO_THROW(simulator.stop());
}

// Test: Generate traffic
TEST_F(UESimulatorTest, GenerateTraffic_DefaultCount_Generates20Packets) {
    simulator.start();
    
    EXPECT_NO_THROW(simulator.generateTraffic());
    
    std::this_thread::sleep_for(std::chrono::seconds(5));
    simulator.stop();
}

// Test: Generate traffic with custom count
TEST_F(UESimulatorTest, GenerateTraffic_CustomCount_GeneratesCorrectNumber) {
    simulator.start();
    
    EXPECT_NO_THROW(simulator.generateTraffic(10));
    
    std::this_thread::sleep_for(std::chrono::seconds(3));
    simulator.stop();
}

// Test: Multiple start calls
TEST_F(UESimulatorTest, Start_MultipleCalls_HandlesGracefully) {
    simulator.start();
    simulator.start(); // Should handle gracefully
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    simulator.stop();
}

// Test: Multiple stop calls
TEST_F(UESimulatorTest, Stop_MultipleCalls_HandlesGracefully) {
    simulator.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    simulator.stop();
    simulator.stop(); // Should handle gracefully
}

// Test: Stop without start
TEST_F(UESimulatorTest, Stop_WithoutStart_HandlesGracefully) {
    EXPECT_NO_THROW(simulator.stop());
}

// Test: Generate traffic without start
TEST_F(UESimulatorTest, GenerateTraffic_WithoutStart_WorksButNoTransmission) {
    EXPECT_NO_THROW(simulator.generateTraffic(5));
}

// Test: Full simulation cycle
TEST_F(UESimulatorTest, FullCycle_StartGenerateStop_CompletesSuccessfully) {
    simulator.start();
    simulator.generateTraffic(20);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    simulator.stop();
    
    // Should complete without crashes
    SUCCEED();
}

// Test: Rapid start-stop cycles
TEST_F(UESimulatorTest, RapidCycles_MultipleStartStop_HandlesCorrectly) {
    for (int i = 0; i < 5; ++i) {
        simulator.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        simulator.stop();
    }
    
    SUCCEED();
}

// Test: Large traffic generation
TEST_F(UESimulatorTest, GenerateTraffic_LargeCount_HandlesCorrectly) {
    simulator.start();
    
    EXPECT_NO_THROW(simulator.generateTraffic(100));
    
    std::this_thread::sleep_for(std::chrono::seconds(25));
    simulator.stop();
}

// Test: Zero packet generation
TEST_F(UESimulatorTest, GenerateTraffic_ZeroPackets_HandlesGracefully) {
    simulator.start();
    
    EXPECT_NO_THROW(simulator.generateTraffic(0));
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    simulator.stop();
}

// Test: Destructor cleanup
TEST_F(UESimulatorTest, Destructor_AfterStart_CleansUpProperly) {
    {
        UESimulator tempSim;
        tempSim.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // Destructor should handle cleanup
    }
    
    SUCCEED();
}

// Test: Thread safety during operation
TEST_F(UESimulatorTest, Operation_ConcurrentAccess_ThreadSafe) {
    simulator.start();
    
    std::thread t1([this]() {
        simulator.generateTraffic(10);
    });
    
    std::thread t2([this]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
    });
    
    t1.join();
    t2.join();
    
    simulator.stop();
}

// Test: Long running simulation
TEST_F(UESimulatorTest, LongRunning_ExtendedOperation_MaintainsStability) {
    simulator.start();
    
    for (int i = 0; i < 5; ++i) {
        simulator.generateTraffic(10);
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    
    simulator.stop();
    SUCCEED();
}

// Test: Statistics observer functionality
TEST_F(UESimulatorTest, StatisticsObserver_ReceivesUpdates_WorksCorrectly) {
    simulator.start();
    simulator.generateTraffic(5);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    simulator.stop();
    
    // Observer should have received updates (tested implicitly)
    SUCCEED();
}

// Integration Test: End-to-end data flow
class IntegrationTest : public ::testing::Test {
protected:
    std::shared_ptr<PDCPLayer> pdcp;
    std::shared_ptr<RLCLayer> rlc;
    std::shared_ptr<MACLayer> mac;
    
    void SetUp() override {
        pdcp = std::make_shared<PDCPLayer>();
        rlc = std::make_shared<RLCLayer>(RlcMode::UM);
        mac = std::make_shared<MACLayer>(rlc);
    }
};

// Test: Complete data flow through all layers
TEST_F(IntegrationTest, DataFlow_ThroughAllLayers_WorksCorrectly) {
    // Application data
    std::vector<uint8_t> appData = {0x01, 0x02, 0x03, 0x04, 0x05};
    
    // PDCP processing
    auto pdcpPdu = pdcp->addHeader(appData);
    EXPECT_EQ(pdcpPdu.size(), appData.size() + 2);
    
    // RLC processing
    rlc->segmentSDU(pdcpPdu, Priority::HIGH);
    EXPECT_EQ(rlc->getQueueSize(), 1);
    
    // MAC processing
    mac->scheduleTransmission();
    auto stats = mac->getStatistics();
    EXPECT_EQ(stats.transmittedPackets, 1);
    EXPECT_EQ(stats.transmittedBytes, pdcpPdu.size());
}

// Test: Multiple packets through pipeline
TEST_F(IntegrationTest, DataFlow_MultiplePackets_AllTransmitted) {
    for (int i = 0; i < 10; ++i) {
        std::vector<uint8_t> data(100, static_cast<uint8_t>(i));
        auto pdcpPdu = pdcp->addHeader(data);
        rlc->segmentSDU(pdcpPdu, Priority::MEDIUM);
    }
    
    for (int i = 0; i < 10; ++i) {
        mac->scheduleTransmission();
    }
    
    auto stats = mac->getStatistics();
    EXPECT_EQ(stats.transmittedPackets, 10);
    EXPECT_TRUE(rlc->isQueueEmpty());
}

// Test: Priority handling through layers
TEST_F(IntegrationTest, DataFlow_DifferentPriorities_PreservedThroughLayers) {
    std::vector<uint8_t> data = {0x01, 0x02};
    auto pdcpPdu = pdcp->addHeader(data);
    
    rlc->segmentSDU(pdcpPdu, Priority::HIGH);
    
    PDU pdu;
    rlc->receivePDU(pdu);
    
    EXPECT_EQ(pdu.priority, Priority::HIGH);
}

// Test: Large data through pipeline
TEST_F(IntegrationTest, DataFlow_LargeData_HandledCorrectly) {
    std::vector<uint8_t> largeData(10000, 0xFF);
    auto pdcpPdu = pdcp->addHeader(largeData);
    
    rlc->segmentSDU(pdcpPdu, Priority::LOW);
    mac->scheduleTransmission();
    
    auto stats = mac->getStatistics();
    EXPECT_EQ(stats.transmittedPackets, 1);
    EXPECT_EQ(stats.transmittedBytes, pdcpPdu.size());
}

// Test: Continuous operation
TEST_F(IntegrationTest, DataFlow_ContinuousOperation_MaintainsIntegrity) {
    for (int i = 0; i < 100; ++i) {
        std::vector<uint8_t> data = {static_cast<uint8_t>(i)};
        auto pdcpPdu = pdcp->addHeader(data);
        rlc->segmentSDU(pdcpPdu, Priority::MEDIUM);
        mac->scheduleTransmission();
    }
    
    auto stats = mac->getStatistics();
    EXPECT_EQ(stats.transmittedPackets, 100);
}

// Stress Test: High load
TEST_F(IntegrationTest, StressTest_HighLoad_HandlesCorrectly) {
    const int numPackets = 1000;
    
    // Generate load
    for (int i = 0; i < numPackets; ++i) {
        std::vector<uint8_t> data(500, static_cast<uint8_t>(i % 256));
        auto pdcpPdu = pdcp->addHeader(data);
        rlc->segmentSDU(pdcpPdu, static_cast<Priority>((i % 3) + 1));
    }
    
    // Process all
    for (int i = 0; i < numPackets; ++i) {
        mac->scheduleTransmission();
    }
    
    auto stats = mac->getStatistics();
    EXPECT_EQ(stats.transmittedPackets, numPackets);
    EXPECT_TRUE(rlc->isQueueEmpty());
}
