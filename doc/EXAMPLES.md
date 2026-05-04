# Usage Examples - UE Simulator 5G

This document provides practical examples of using the UE Simulator.

---

## Basic Usage

### Example 1: Simple Simulation

```cpp
#include "ue_simulator.hpp"

int main() {
    // Create simulator instance
    ue_simulator::UESimulator simulator;
    
    // Start the simulator (spawns threads)
    simulator.start();
    
    // Generate 20 packets with random sizes and priorities
    simulator.generateTraffic(20);
    
    // Wait for processing
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Stop simulator (joins threads)
    simulator.stop();
    
    return 0;
}
```

**Output:**
```
╔════════════════════════════════════════╗
║   UE SIMULATOR 5G - L2 STACK          ║
║   3GPP TS 38.321/322/323 Compliant    ║
╚════════════════════════════════════════╝

[SYSTEM] Generating traffic...
[RLC] Segmented PDU ID=0 size=1245B priority=2
[MAC] Transmitted PDU ID=0 latency=15ms
...
```

---

## Advanced Usage

### Example 2: Custom Traffic Generation

```cpp
#include "ue_simulator.hpp"
#include "common_types.hpp"

int main() {
    ue_simulator::UESimulator simulator;
    simulator.start();
    
    // Access individual layers (if needed for advanced use)
    auto pdcpLayer = std::make_shared<ue_simulator::PDCPLayer>();
    auto rlcLayer = std::make_shared<ue_simulator::RLCLayer>(
        ue_simulator::RlcMode::UM
    );
    
    // Generate high-priority traffic
    for (int i = 0; i < 10; i++) {
        std::vector<uint8_t> data(1000, 0xAB);
        auto pdu = pdcpLayer->addHeader(data);
        rlcLayer->segmentSDU(pdu, ue_simulator::Priority::HIGH);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    simulator.stop();
    return 0;
}
```

---

### Example 3: Statistics Monitoring

```cpp
#include "ue_simulator.hpp"

class CustomObserver : public ue_simulator::IStatisticsObserver {
public:
    void onStatisticsUpdate(const ue_simulator::Statistics& stats) override {
        std::cout << "Custom monitoring:\n";
        std::cout << "  Packets: " << stats.transmittedPackets << "\n";
        std::cout << "  Bytes: " << stats.transmittedBytes << "\n";
        std::cout << "  Queue: " << stats.queueSize << "\n";
    }
};

int main() {
    ue_simulator::UESimulator simulator;
    
    // Register custom observer
    auto observer = std::make_shared<CustomObserver>();
    // Note: In production, you'd register this with MAC layer
    
    simulator.start();
    simulator.generateTraffic(50);
    
    std::this_thread::sleep_for(std::chrono::seconds(5));
    simulator.stop();
    
    return 0;
}
```

---

### Example 4: Different RLC Modes

```cpp
#include "rlc_layer.hpp"

int main() {
    // Transparent Mode
    auto rlcTM = std::make_shared<ue_simulator::RLCLayer>(
        ue_simulator::RlcMode::TM
    );
    
    // Unacknowledged Mode (default)
    auto rlcUM = std::make_shared<ue_simulator::RLCLayer>(
        ue_simulator::RlcMode::UM
    );
    
    // Acknowledged Mode (future implementation)
    auto rlcAM = std::make_shared<ue_simulator::RLCLayer>(
        ue_simulator::RlcMode::AM
    );
    
    return 0;
}
```

---

## Layer-by-Layer Examples

### PDCP Layer Usage

```cpp
#include "pdcp_layer.hpp"

void pdcpExample() {
    ue_simulator::PDCPLayer pdcp;
    
    // Create SDU (Service Data Unit)
    std::vector<uint8_t> sdu = {0x01, 0x02, 0x03, 0x04};
    
    // Add PDCP header
    auto pdu = pdcp.addHeader(sdu);
    // Result: [0xAA, 0x01, 0x02, 0x03, 0x04, 0xFF]
    
    // Remove PDCP header
    auto originalSdu = pdcp.removeHeader(pdu);
    // Result: [0x01, 0x02, 0x03, 0x04]
    
    // Get statistics
    auto stats = pdcp.getStatistics();
    std::cout << "PDCP transmitted: " << stats.transmittedPackets << "\n";
}
```

---

### RLC Layer Usage

```cpp
#include "rlc_layer.hpp"

void rlcExample() {
    ue_simulator::RLCLayer rlc(ue_simulator::RlcMode::UM);
    
    // Segment data with priority
    std::vector<uint8_t> data(1500, 0xFF);
    rlc.segmentSDU(data, ue_simulator::Priority::HIGH);
    
    // Check queue status
    std::cout << "Queue size: " << rlc.getQueueSize() << "\n";
    std::cout << "Is empty: " << rlc.isQueueEmpty() << "\n";
    
    // Retrieve PDU
    ue_simulator::PDU pdu;
    if (rlc.receivePDU(pdu)) {
        std::cout << "Retrieved PDU ID: " << pdu.id << "\n";
        std::cout << "Priority: " << static_cast<int>(pdu.priority) << "\n";
    }
}
```

---

### MAC Layer Usage

```cpp
#include "mac_layer.hpp"
#include "rlc_layer.hpp"

void macExample() {
    auto rlc = std::make_shared<ue_simulator::RLCLayer>(
        ue_simulator::RlcMode::UM
    );
    ue_simulator::MACLayer mac(rlc);
    
    // Add some data to RLC
    std::vector<uint8_t> data(1000, 0xAB);
    rlc->segmentSDU(data, ue_simulator::Priority::MEDIUM);
    
    // Schedule transmission
    mac.scheduleTransmission();
    
    // Get statistics
    auto stats = mac.getStatistics();
    std::cout << "MAC Statistics:\n";
    std::cout << "  Packets: " << stats.transmittedPackets << "\n";
    std::cout << "  Bytes: " << stats.transmittedBytes << "\n";
    std::cout << "  Queue: " << stats.queueSize << "\n";
}
```

---

## Performance Testing

### Example 5: High Load Test

```cpp
#include "ue_simulator.hpp"
#include <chrono>

void performanceTest() {
    ue_simulator::UESimulator simulator;
    simulator.start();
    
    auto startTime = std::chrono::steady_clock::now();
    
    // Generate 1000 packets
    simulator.generateTraffic(1000);
    
    // Wait for completion
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(
        endTime - startTime
    ).count();
    
    std::cout << "Test duration: " << duration << " seconds\n";
    
    simulator.stop();
}
```

---

### Example 6: Latency Measurement

```cpp
#include "ue_simulator.hpp"

void latencyTest() {
    auto rlc = std::make_shared<ue_simulator::RLCLayer>(
        ue_simulator::RlcMode::UM
    );
    auto mac = std::make_shared<ue_simulator::MACLayer>(rlc);
    
    std::vector<uint64_t> latencies;
    
    for (int i = 0; i < 100; i++) {
        std::vector<uint8_t> data(1000, 0xFF);
        
        auto startTime = std::chrono::steady_clock::now();
        rlc->segmentSDU(data, ue_simulator::Priority::HIGH);
        mac->scheduleTransmission();
        auto endTime = std::chrono::steady_clock::now();
        
        auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime
        ).count();
        
        latencies.push_back(latency);
    }
    
    // Calculate average latency
    uint64_t sum = 0;
    for (auto lat : latencies) sum += lat;
    std::cout << "Average latency: " << (sum / latencies.size()) << "ms\n";
}
```

---

## Error Handling

### Example 7: Exception Handling

```cpp
#include "ue_simulator.hpp"

int main() {
    try {
        ue_simulator::UESimulator simulator;
        simulator.start();
        simulator.generateTraffic(20);
        
        std::this_thread::sleep_for(std::chrono::seconds(2));
        simulator.stop();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred\n";
        return 2;
    }
    
    return 0;
}
```

---

## Integration Examples

### Example 8: Integration with External System

```cpp
#include "ue_simulator.hpp"
#include <fstream>

class FileLogger : public ue_simulator::IStatisticsObserver {
private:
    std::ofstream logFile_;
    
public:
    FileLogger(const std::string& filename) 
        : logFile_(filename, std::ios::app) {}
    
    void onStatisticsUpdate(const ue_simulator::Statistics& stats) override {
        logFile_ << "Timestamp: " << std::time(nullptr) << "\n";
        logFile_ << "Packets: " << stats.transmittedPackets << "\n";
        logFile_ << "Bytes: " << stats.transmittedBytes << "\n";
        logFile_ << "Queue: " << stats.queueSize << "\n";
        logFile_ << "---\n";
        logFile_.flush();
    }
};

int main() {
    ue_simulator::UESimulator simulator;
    auto logger = std::make_shared<FileLogger>("simulation.log");
    
    // Register logger (in production code)
    // macLayer->registerObserver(logger);
    
    simulator.start();
    simulator.generateTraffic(100);
    
    std::this_thread::sleep_for(std::chrono::seconds(5));
    simulator.stop();
    
    return 0;
}
```

---

## Configuration Examples

### Example 9: Custom Configuration

```cpp
#include "ue_simulator.hpp"

struct SimulatorConfig {
    size_t numPackets = 20;
    size_t minPacketSize = 64;
    size_t maxPacketSize = 1500;
    int transmissionDelayMs = 10;
    int schedulingIntervalMs = 50;
};

void runWithConfig(const SimulatorConfig& config) {
    ue_simulator::UESimulator simulator;
    simulator.start();
    
    // Generate traffic according to config
    simulator.generateTraffic(config.numPackets);
    
    std::this_thread::sleep_for(std::chrono::seconds(5));
    simulator.stop();
}

int main() {
    SimulatorConfig config;
    config.numPackets = 50;
    config.minPacketSize = 128;
    config.maxPacketSize = 2000;
    
    runWithConfig(config);
    return 0;
}
```

---

## Testing Examples

### Example 10: Unit Test Structure (Google Test)

```cpp
#include <gtest/gtest.h>
#include "pdcp_layer.hpp"

class PDCPLayerTest : public ::testing::Test {
protected:
    ue_simulator::PDCPLayer pdcp;
};

TEST_F(PDCPLayerTest, AddHeaderTest) {
    std::vector<uint8_t> sdu = {0x01, 0x02, 0x03};
    auto pdu = pdcp.addHeader(sdu);
    
    EXPECT_EQ(pdu.size(), sdu.size() + 2);
    EXPECT_EQ(pdu.front(), 0xAA);
    EXPECT_EQ(pdu.back(), 0xFF);
}

TEST_F(PDCPLayerTest, RemoveHeaderTest) {
    std::vector<uint8_t> pdu = {0xAA, 0x01, 0x02, 0x03, 0xFF};
    auto sdu = pdcp.removeHeader(pdu);
    
    EXPECT_EQ(sdu.size(), 3);
    EXPECT_EQ(sdu[0], 0x01);
    EXPECT_EQ(sdu[2], 0x03);
}
```

---

## Command Line Interface (Future)

### Example 11: CLI Usage

```bash
# Run with default settings
./ue_simulator

# Run with custom packet count
./ue_simulator --packets 100

# Run with specific RLC mode
./ue_simulator --rlc-mode UM

# Run with verbose output
./ue_simulator --verbose

# Run with configuration file
./ue_simulator --config config.json
```

---

## Best Practices

### 1. Always Use RAII
```cpp
{
    ue_simulator::UESimulator simulator;
    simulator.start();
    // ... use simulator ...
    // Destructor automatically calls stop()
}
```

### 2. Check Return Values
```cpp
ue_simulator::PDU pdu;
if (rlcLayer->receivePDU(pdu)) {
    // Process PDU
} else {
    // Queue is empty
}
```

### 3. Use Smart Pointers
```cpp
auto rlc = std::make_shared<ue_simulator::RLCLayer>(
    ue_simulator::RlcMode::UM
);
// Automatic memory management
```

### 4. Handle Exceptions
```cpp
try {
    simulator.start();
} catch (const std::exception& e) {
    std::cerr << "Failed to start: " << e.what() << "\n";
}
```

---

## Troubleshooting

### Issue: Simulator hangs on stop()
**Solution**: Ensure threads are properly joined
```cpp
simulator.stop(); // Waits for threads to finish
```

### Issue: Queue overflow
**Solution**: Check queue size before adding
```cpp
if (rlc->getQueueSize() < 1000) {
    rlc->segmentSDU(data, priority);
}
```

### Issue: High latency
**Solution**: Reduce scheduling interval or increase thread priority
```cpp
// Modify scheduling interval in transmissionLoop()
std::this_thread::sleep_for(std::chrono::milliseconds(25)); // Faster
```

---

## Performance Tips

1. **Use move semantics** for large data transfers
2. **Pre-allocate vectors** with reserve()
3. **Minimize lock contention** with short critical sections
4. **Use atomic operations** for simple counters
5. **Profile with tools** like Valgrind, perf

---

## Conclusion

These examples demonstrate the flexibility and power of the UE Simulator. The clean architecture and SOLID principles make it easy to extend and customize for specific use cases.

For more information, see:
- [README.md](README.md) - General documentation
- [ARCHITECTURE.md](ARCHITECTURE.md) - Design details
- 3GPP specifications (TS 38.321, 38.322, 38.323)
