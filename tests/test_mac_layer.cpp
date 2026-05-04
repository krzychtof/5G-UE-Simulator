#include <gtest/gtest.h>
#include "mac_layer.hpp"
#include "rlc_layer.hpp"
#include <memory>
#include <thread>

using namespace ue_simulator;

class MACLayerTest : public ::testing::Test {
protected:
    std::shared_ptr<RLCLayer> rlc;
    std::shared_ptr<MACLayer> mac;
    
    void SetUp() override {
        rlc = std::make_shared<RLCLayer>(RlcMode::UM);
        mac = std::make_shared<MACLayer>(rlc);
    }
};

// Test: MAC layer construction
TEST_F(MACLayerTest, Constructor_ValidRLC_CreatesSuccessfully) {
    EXPECT_NE(mac, nullptr);
}

// Test: Schedule transmission with empty queue
TEST_F(MACLayerTest, ScheduleTransmission_EmptyQueue_NoTransmission) {
    auto statsBefore = mac->getStatistics();
    
    mac->scheduleTransmission();
    
    auto statsAfter = mac->getStatistics();
    EXPECT_EQ(statsBefore.transmittedPackets, statsAfter.transmittedPackets);
}

// Test: Schedule transmission with data
TEST_F(MACLayerTest, ScheduleTransmission_WithData_TransmitsSuccessfully) {
    rlc->segmentSDU({0x01, 0x02, 0x03}, Priority::HIGH);
    
    mac->scheduleTransmission();
    
    auto stats = mac->getStatistics();
    EXPECT_EQ(stats.transmittedPackets, 1);
    EXPECT_EQ(stats.transmittedBytes, 3);
}

// Test: Multiple transmissions
TEST_F(MACLayerTest, ScheduleTransmission_MultiplePDUs_TransmitsAll) {
    rlc->segmentSDU({0x01}, Priority::LOW);
    rlc->segmentSDU({0x02, 0x03}, Priority::MEDIUM);
    rlc->segmentSDU({0x04, 0x05, 0x06}, Priority::HIGH);
    
    mac->scheduleTransmission();
    mac->scheduleTransmission();
    mac->scheduleTransmission();
    
    auto stats = mac->getStatistics();
    EXPECT_EQ(stats.transmittedPackets, 3);
    EXPECT_EQ(stats.transmittedBytes, 6);
}

// Test: Statistics accuracy
TEST_F(MACLayerTest, GetStatistics_AfterTransmissions_ReturnsAccurateCounts) {
    for (int i = 0; i < 10; ++i) {
        rlc->segmentSDU({0x01, 0x02}, Priority::LOW);
        mac->scheduleTransmission();
    }
    
    auto stats = mac->getStatistics();
    EXPECT_EQ(stats.transmittedPackets, 10);
    EXPECT_EQ(stats.transmittedBytes, 20);
}

// Test: Queue size reporting
TEST_F(MACLayerTest, GetStatistics_QueueSize_ReflectsRLCQueue) {
    rlc->segmentSDU({0x01}, Priority::LOW);
    rlc->segmentSDU({0x02}, Priority::LOW);
    rlc->segmentSDU({0x03}, Priority::LOW);
    
    auto stats = mac->getStatistics();
    EXPECT_EQ(stats.queueSize, 3);
    
    mac->scheduleTransmission();
    stats = mac->getStatistics();
    EXPECT_EQ(stats.queueSize, 2);
}

// Test: Observer registration
TEST_F(MACLayerTest, RegisterObserver_ValidObserver_RegistersSuccessfully) {
    class TestObserver : public IStatisticsObserver {
    public:
        int updateCount = 0;
        void onStatisticsUpdate(const Statistics&) override {
            updateCount++;
        }
    };
    
    auto observer = std::make_shared<TestObserver>();
    mac->registerObserver(observer);
    
    rlc->segmentSDU({0x01}, Priority::LOW);
    mac->scheduleTransmission();
    
    EXPECT_EQ(observer->updateCount, 1);
}

// Test: Observer notification on transmission
TEST_F(MACLayerTest, ScheduleTransmission_WithObserver_NotifiesObserver) {
    class TestObserver : public IStatisticsObserver {
    public:
        Statistics lastStats;
        void onStatisticsUpdate(const Statistics& stats) override {
            lastStats = stats;
        }
    };
    
    auto observer = std::make_shared<TestObserver>();
    mac->registerObserver(observer);
    
    rlc->segmentSDU({0x01, 0x02, 0x03}, Priority::HIGH);
    mac->scheduleTransmission();
    
    EXPECT_EQ(observer->lastStats.transmittedPackets, 1);
    EXPECT_EQ(observer->lastStats.transmittedBytes, 3);
}

// Test: Latency measurement
TEST_F(MACLayerTest, ScheduleTransmission_MeasuresLatency_TakesTime) {
    rlc->segmentSDU({0x01}, Priority::LOW);
    
    auto start = std::chrono::steady_clock::now();
    mac->scheduleTransmission();
    auto end = std::chrono::steady_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    EXPECT_GE(duration, 10); // At least 10ms transmission delay
}

// Test: Thread safety - concurrent transmissions
TEST_F(MACLayerTest, ScheduleTransmission_ConcurrentCalls_ThreadSafe) {
    // Fill queue
    for (int i = 0; i < 100; ++i) {
        rlc->segmentSDU({0x01, 0x02}, Priority::LOW);
    }
    
    const int numThreads = 5;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this]() {
            for (int j = 0; j < 20; ++j) {
                mac->scheduleTransmission();
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto stats = mac->getStatistics();
    EXPECT_EQ(stats.transmittedPackets, 100);
}

// Test: Statistics thread safety
TEST_F(MACLayerTest, GetStatistics_ConcurrentAccess_ThreadSafe) {
    std::atomic<bool> running{true};
    
    // Writer thread
    std::thread writer([this, &running]() {
        while (running) {
            rlc->segmentSDU({0x01}, Priority::LOW);
            mac->scheduleTransmission();
        }
    });
    
    // Reader threads
    std::vector<std::thread> readers;
    for (int i = 0; i < 5; ++i) {
        readers.emplace_back([this, &running]() {
            while (running) {
                auto stats = mac->getStatistics();
                EXPECT_GE(stats.transmittedPackets, 0);
            }
        });
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    running = false;
    
    writer.join();
    for (auto& r : readers) {
        r.join();
    }
}

// Test: Large data transmission
TEST_F(MACLayerTest, ScheduleTransmission_LargeData_HandlesCorrectly) {
    std::vector<uint8_t> largeData(10000, 0xAB);
    rlc->segmentSDU(largeData, Priority::HIGH);
    
    mac->scheduleTransmission();
    
    auto stats = mac->getStatistics();
    EXPECT_EQ(stats.transmittedPackets, 1);
    EXPECT_EQ(stats.transmittedBytes, 10000);
}

// Test: Empty PDU transmission
TEST_F(MACLayerTest, ScheduleTransmission_EmptyPDU_HandlesCorrectly) {
    rlc->segmentSDU({}, Priority::LOW);
    
    mac->scheduleTransmission();
    
    auto stats = mac->getStatistics();
    EXPECT_EQ(stats.transmittedPackets, 1);
    EXPECT_EQ(stats.transmittedBytes, 0);
}

// Test: Continuous operation
TEST_F(MACLayerTest, ScheduleTransmission_ContinuousOperation_MaintainsAccuracy) {
    for (int i = 0; i < 1000; ++i) {
        rlc->segmentSDU({static_cast<uint8_t>(i % 256)}, Priority::MEDIUM);
        mac->scheduleTransmission();
    }
    
    auto stats = mac->getStatistics();
    EXPECT_EQ(stats.transmittedPackets, 1000);
    EXPECT_EQ(stats.transmittedBytes, 1000);
}

// Test: Priority handling (FIFO for now)
TEST_F(MACLayerTest, ScheduleTransmission_DifferentPriorities_TransmitsInOrder) {
    rlc->segmentSDU({0x01}, Priority::LOW);
    rlc->segmentSDU({0x02}, Priority::HIGH);
    rlc->segmentSDU({0x03}, Priority::MEDIUM);
    
    mac->scheduleTransmission();
    mac->scheduleTransmission();
    mac->scheduleTransmission();
    
    auto stats = mac->getStatistics();
    EXPECT_EQ(stats.transmittedPackets, 3);
}

// Test: Receive PDU interface
TEST_F(MACLayerTest, ReceivePDU_WithDataInQueue_ReturnsPDU) {
    rlc->segmentSDU({0x01, 0x02, 0x03}, Priority::HIGH);
    
    PDU pdu;
    bool result = mac->receivePDU(pdu);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(pdu.data.size(), 3);
}

// Test: Receive PDU from empty queue
TEST_F(MACLayerTest, ReceivePDU_EmptyQueue_ReturnsFalse) {
    PDU pdu;
    bool result = mac->receivePDU(pdu);
    
    EXPECT_FALSE(result);
}

// Test: Send PDU interface (not used in current implementation)
TEST_F(MACLayerTest, SendPDU_AnyPDU_ReturnsFalse) {
    PDU pdu;
    pdu.data = {0x01, 0x02};
    
    bool result = mac->sendPDU(pdu);
    
    EXPECT_FALSE(result);
}
