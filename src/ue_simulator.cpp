#include "ue_simulator.hpp"
#include <iostream>
#include <random>

namespace ue_simulator {

UESimulator::UESimulator() : running_(false) {
    // Dependency Injection - create protocol stack
    // Creates PDCP (Packet Data Convergence Protocol) layer - responsible for header compression, ciphering and integrity protection
    pdcpLayer_ = std::make_shared<PDCPLayer>();
    // Creates RLC (Radio Link Control) layer in UM (Unacknowledged Mode) - responsible for segmentation and reassembly
    rlcLayer_ = std::make_shared<RLCLayer>(RlcMode::UM);
    // Creates MAC (Medium Access Control) layer with RLC layer reference - manages access to the radio medium
    macLayer_ = std::make_shared<MACLayer>(rlcLayer_);
    
    // Register observer (Observer pattern)
    // Registers this object as a MAC statistics observer - Observer pattern for monitoring changes
    macLayer_->registerObserver(std::shared_ptr<IStatisticsObserver>(this, [](IStatisticsObserver*){}));
}

UESimulator::~UESimulator() {
    // Destructor - stops the simulator before destroying the object
    stop();
}

void UESimulator::start() {
    // Check if simulator is already running - if so, return early
    if (running_.load()) {
        return;
    }
    
    // Print startup banner
    printBanner();
    // Set running flag to true (atomically)
    running_ = true;
    
    // Transmission thread
    // Start thread responsible for the transmission loop
    txThread_ = std::thread(&UESimulator::transmissionLoop, this);
    
    // Statistics thread
    // Start thread responsible for periodic statistics display
    statsThread_ = std::thread(&UESimulator::statisticsLoop, this);
}

void UESimulator::stop() {
    // Check if simulator is not running - if so, return early
    if (!running_.load()) {
        return;
    }
    
    // Set running flag to false - signals threads to finish
    running_ = false;
    
    // Wait for transmission thread to finish if joinable
    if (txThread_.joinable()) {
        txThread_.join();
    }
    
    // Wait for statistics thread to finish if joinable
    if (statsThread_.joinable()) {
        statsThread_.join();
    }
    
    // Print simulator stopped message
    std::cout << "\n[SYSTEM] Simulator stopped\n";
    
    // Print final statistics
    // Retrieve final statistics from MAC layer
    auto stats = macLayer_->getStatistics();
    // Print final summary with transmitted packets, bytes and queue size
    std::cout << "\n=== FINAL STATISTICS ===\n";
    std::cout << "Transmitted packets: " << stats.transmittedPackets << "\n";
    std::cout << "Total bytes: " << stats.transmittedBytes << "\n";
    std::cout << "Queue size: " << stats.queueSize << "\n";
    std::cout << "========================\n";
}

void UESimulator::generateTraffic(size_t numPackets) {
    // Initialize random number generator with true entropy source
    std::random_device rd;
    std::mt19937 gen(rd());
    // Random distribution for packet size (64-1500 bytes) - typical sizes for 5G networks
    std::uniform_int_distribution<> sizeDist(64, 1500);
    // Random distribution for priority (1-3) - different QoS classes
    std::uniform_int_distribution<> prioDist(1, 3);
    
    // Print traffic generation start message
    std::cout << "[SYSTEM] Generating traffic...\n\n";
    
    // Loop generating the specified number of packets
    for (size_t i = 0; i < numPackets; ++i) {
        // Generate random data
        // Draw packet size from the defined range
        size_t size = sizeDist(gen);
        // Draw priority and cast to Priority type
        Priority priority = static_cast<Priority>(prioDist(gen));
        
        // Create byte vector of the drawn size
        std::vector<uint8_t> data(size);
        // Fill each byte with a random value (0-255)
        for (auto& byte : data) {
            byte = static_cast<uint8_t>(gen() % 256);
        }
        
        // PDCP processing
        // Process data through PDCP layer - add PDCP header
        auto pdcpPdu = pdcpLayer_->addHeader(data);
        
        // RLC segmentation
        // Segment SDU (Service Data Unit) through RLC layer with priority
        rlcLayer_->segmentSDU(pdcpPdu, priority);
        
        // 200ms delay between packet generation - simulates real traffic
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void UESimulator::onStatisticsUpdate(const Statistics& stats) {
    (void)stats; // Observer callback - can be extended for real-time monitoring
}

void UESimulator::transmissionLoop() {
    // Main transmission thread loop - runs while simulator is active
    while (running_.load()) {
        // Invoke MAC layer transmission scheduling
        macLayer_->scheduleTransmission();
        // 50ms delay between scheduling cycles - simulates TTI (Transmission Time Interval)
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void UESimulator::statisticsLoop() {
    // Main statistics thread loop - runs while simulator is active
    while (running_.load()) {
        // Wait 3 seconds before next statistics display
        std::this_thread::sleep_for(std::chrono::seconds(3));
        
        // Retrieve current statistics from MAC layer
        auto stats = macLayer_->getStatistics();
        // Print formatted statistics: packet count, bytes, RLC queue size
        std::cout << "\n=== MAC STATISTICS ===\n";
        std::cout << "Transmitted packets: " << stats.transmittedPackets << "\n";
        std::cout << "Total bytes: " << stats.transmittedBytes << "\n";
        std::cout << "RLC queue size: " << stats.queueSize << "\n";
        std::cout << "======================\n\n";
    }
}

void UESimulator::printBanner() const {
    // Print startup banner with simulator info and 3GPP compliance
    std::cout << "\n╔════════════════════════════════════════╗\n";
    std::cout << "║   UE SIMULATOR 5G - L2 STACK          ║\n";
    std::cout << "║   3GPP TS 38.321/322/323 Compliant    ║\n";
    std::cout << "╚════════════════════════════════════════╝\n\n";
}

} // namespace ue_simulator
