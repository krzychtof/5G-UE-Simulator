#include "rlc_layer.hpp"
#include <iostream>

namespace ue_simulator {

RLCLayer::RLCLayer(RlcMode mode) 
    : mode_(mode), pduCounter_(0) {}

bool RLCLayer::sendPDU(const PDU& pdu) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (txQueue_.size() >= MAX_QUEUE_SIZE) {
        stats_.droppedPackets++;
        return false;
    }
    
    txQueue_.push(pdu);
    stats_.queueSize = txQueue_.size();
    cv_.notify_one();
    
    std::cout << "[RLC] Segmented PDU ID=" << pdu.id 
              << " size=" << pdu.data.size() << "B"
              << " priority=" << static_cast<int>(pdu.priority) << "\n";
    
    return true;
}

bool RLCLayer::receivePDU(PDU& pdu) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (txQueue_.empty()) {
        return false;
    }
    
    pdu = txQueue_.front();
    txQueue_.pop();
    stats_.queueSize = txQueue_.size();
    
    return true;
}

Statistics RLCLayer::getStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

void RLCLayer::segmentSDU(const std::vector<uint8_t>& sdu, Priority priority) {
    PDU pdu;
    pdu.id = pduCounter_++;
    pdu.data = sdu;
    pdu.priority = priority;
    pdu.timestamp = std::chrono::steady_clock::now();
    
    sendPDU(pdu);
}

size_t RLCLayer::getQueueSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return txQueue_.size();
}

bool RLCLayer::isQueueEmpty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return txQueue_.empty();
}

} // namespace ue_simulator
