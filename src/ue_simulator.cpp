#include "ue_simulator.hpp"
#include <iostream>
#include <random>

namespace ue_simulator {

UESimulator::UESimulator() : running_(false) {
    // Dependency Injection - create protocol stack
    pdcpLayer_ = std::make_shared<PDCPLayer>();
    rlcLayer_ = std::make_shared<RLCLayer>(RlcMode::UM);
    macLayer_ = std::make_shared<MACLayer>(rlcLayer_);
    
    // Register observer (Observer pattern)
    macLayer_->registerObserver(std::shared_ptr<IStatisticsObserver>(this, [](IStatisticsObserver*){}));
}

UESimulator::~UESimulator() {
    stop();
}

void UESimulator::start() {
    if (running_.load()) {
        return;
    }
    
    printBanner();
    running_ = true;
    
    // Transmission thread
    txThread_ = std::thread(&UESimulator::transmissionLoop, this);
    
    // Statistics thread
    statsThread_ = std::thread(&UESimulator::statisticsLoop, this);
}

void UESimulator::stop() {
    if (!running_.load()) {
        return;
    }
    
    running_ = false;
    
    if (txThread_.joinable()) {
        txThread_.join();
    }
    
    if (statsThread_.joinable()) {
        statsThread_.join();
    }
    
    std::cout << "\n[SYSTEM] Simulator stopped\n";
    
    // Print final statistics
    auto stats = macLayer_->getStatistics();
    std::cout << "\n=== FINAL STATISTICS ===\n";
    std::cout << "Transmitted packets: " << stats.transmittedPackets << "\n";
    std::cout << "Total bytes: " << stats.transmittedBytes << "\n";
    std::cout << "Queue size: " << stats.queueSize << "\n";
    std::cout << "========================\n";
}

void UESimulator::generateTraffic(size_t numPackets) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> sizeDist(64, 1500);
    std::uniform_int_distribution<> prioDist(1, 3);
    
    std::cout << "[SYSTEM] Generating traffic...\n\n";
    
    for (size_t i = 0; i < numPackets; ++i) {
        // Generate random data
        size_t size = sizeDist(gen);
        Priority priority = static_cast<Priority>(prioDist(gen));
        
        std::vector<uint8_t> data(size);
        for (auto& byte : data) {
            byte = static_cast<uint8_t>(gen() % 256);
        }
        
        // PDCP processing
        auto pdcpPdu = pdcpLayer_->addHeader(data);
        
        // RLC segmentation
        rlcLayer_->segmentSDU(pdcpPdu, priority);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void UESimulator::onStatisticsUpdate(const Statistics& stats) {
    (void)stats; // Observer callback - can be extended for real-time monitoring
}

void UESimulator::transmissionLoop() {
    while (running_.load()) {
        macLayer_->scheduleTransmission();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void UESimulator::statisticsLoop() {
    while (running_.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        
        auto stats = macLayer_->getStatistics();
        std::cout << "\n=== MAC STATISTICS ===\n";
        std::cout << "Transmitted packets: " << stats.transmittedPackets << "\n";
        std::cout << "Total bytes: " << stats.transmittedBytes << "\n";
        std::cout << "RLC queue size: " << stats.queueSize << "\n";
        std::cout << "======================\n\n";
    }
}

void UESimulator::printBanner() const {
    std::cout << "\n╔════════════════════════════════════════╗\n";
    std::cout << "║   UE SIMULATOR 5G/6G - L2 STACK       ║\n";
    std::cout << "║   3GPP TS 38.321/322/323 Compliant    ║\n";
    std::cout << "╚════════════════════════════════════════╝\n\n";
}

} // namespace ue_simulator
