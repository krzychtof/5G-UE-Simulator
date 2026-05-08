#include "rlc_layer.hpp"
#include <iostream>

namespace ue_simulator {

// Constructor: initializes RLC mode and resets PDU counter
RLCLayer::RLCLayer(RlcMode mode) 
    : mode_(mode), pduCounter_(0) {}

bool RLCLayer::sendPDU(const PDU& pdu) {
    std::lock_guard<std::mutex> lock(mutex_); // lock queue for thread-safe access
    
    if (txQueue_.size() >= MAX_QUEUE_SIZE) { // drop PDU if queue is full
        stats_.droppedPackets++;             // increment dropped packet counter
        return false;
    }
    
    txQueue_.push(pdu);                      // enqueue PDU at the back
    stats_.queueSize = txQueue_.size();      // update queue size statistic
    cv_.notify_one();                        // wake up any waiting consumer thread
    
    std::cout << "[RLC] Segmented PDU ID=" << pdu.id 
              << " size=" << pdu.data.size() << "B"
              << " priority=" << static_cast<int>(pdu.priority) << "\n";
    
    return true;
}

bool RLCLayer::receivePDU(PDU& pdu) {
    std::lock_guard<std::mutex> lock(mutex_); // lock queue for thread-safe access
    
    if (txQueue_.empty()) {  // nothing to dequeue
        return false;
    }
    
    pdu = txQueue_.front();             // copy front PDU to output parameter
    txQueue_.pop();                     // remove it from the queue
    stats_.queueSize = txQueue_.size(); // update queue size statistic
    
    return true;
}

Statistics RLCLayer::getStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_); // lock for consistent read
    return stats_;
}

void RLCLayer::segmentSDU(const std::vector<uint8_t>& sdu, Priority priority) {
    PDU pdu;
    pdu.id = pduCounter_++;                          // assign unique ID and increment counter
    pdu.data = sdu;                                  // copy SDU payload into PDU
    pdu.priority = priority;                         // set scheduling priority
    pdu.timestamp = std::chrono::steady_clock::now(); // record creation time for latency tracking
    
    sendPDU(pdu); // enqueue the constructed PDU
}

size_t RLCLayer::getQueueSize() const {
    std::lock_guard<std::mutex> lock(mutex_); // lock for consistent read
    return txQueue_.size();
}

bool RLCLayer::isQueueEmpty() const {
    std::lock_guard<std::mutex> lock(mutex_); // lock for consistent read
    return txQueue_.empty();
}

} // namespace ue_simulator
