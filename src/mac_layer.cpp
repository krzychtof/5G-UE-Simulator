#include "mac_layer.hpp"
#include <iostream>
#include <thread>

namespace ue_simulator {

MACLayer::MACLayer(std::shared_ptr<RLCLayer> rlcLayer)
    : rlcLayer_(rlcLayer), transmittedPackets_(0), transmittedBytes_(0) {}

bool MACLayer::sendPDU(const PDU& pdu) {
    (void)pdu; // Unused in prototype
    return false;
}

bool MACLayer::receivePDU(PDU& pdu) {
    return rlcLayer_->receivePDU(pdu);
}

Statistics MACLayer::getStatistics() const {
    Statistics stats;
    stats.transmittedPackets = transmittedPackets_.load();
    stats.transmittedBytes = transmittedBytes_.load();
    stats.queueSize = rlcLayer_->getQueueSize();
    return stats;
}

void MACLayer::scheduleTransmission() {
    PDU pdu;
    
    if (receivePDU(pdu)) {
        // 3GPP TS 38.321 - MAC scheduling and transmission
        std::this_thread::sleep_for(std::chrono::milliseconds(TRANSMISSION_DELAY_MS));
        
        transmittedPackets_++;
        transmittedBytes_ += pdu.data.size();
        
        auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - pdu.timestamp).count();
        
        std::cout << "[MAC] Transmitted PDU ID=" << pdu.id 
                  << " latency=" << latency << "ms\n";
        
        notifyObserver();
    }
}

void MACLayer::registerObserver(std::shared_ptr<IStatisticsObserver> observer) {
    observer_ = observer;
}

void MACLayer::notifyObserver() {
    if (observer_) {
        observer_->onStatisticsUpdate(getStatistics());
    }
}

} // namespace ue_simulator
