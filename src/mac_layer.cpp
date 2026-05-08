#include "mac_layer.hpp"     // MAC layer class definition
#include <iostream>          // std::cout for console output
#include <thread>            // std::this_thread::sleep_for for transmission delay

namespace ue_simulator {

MACLayer::MACLayer(std::shared_ptr<RLCLayer> rlcLayer)
    : rlcLayer_(rlcLayer),       // store reference to RLC layer for PDU dequeuing
      transmittedPackets_(0),    // initialize packet counter to zero
      transmittedBytes_(0) {}    // initialize byte counter to zero

bool MACLayer::sendPDU(const PDU& pdu) {
    (void)pdu; // suppress unused parameter warning - downlink send not implemented in this prototype
    return false; // always returns false as uplink-only prototype
}

bool MACLayer::receivePDU(PDU& pdu) {
    return rlcLayer_->receivePDU(pdu); // delegate dequeue to RLC layer, fills pdu if available
}

Statistics MACLayer::getStatistics() const {
    Statistics stats;
    stats.transmittedPackets = transmittedPackets_.load(); // atomic read of packet counter
    stats.transmittedBytes = transmittedBytes_.load();     // atomic read of byte counter
    stats.queueSize = rlcLayer_->getQueueSize();           // current number of PDUs waiting in RLC queue
    return stats;
}

void MACLayer::scheduleTransmission() {
    PDU pdu; // PDU to be filled by RLC layer
    
    if (receivePDU(pdu)) { // try to dequeue next PDU from RLC; skip if queue is empty
        // 3GPP TS 38.321 - MAC scheduling and transmission
        std::this_thread::sleep_for(std::chrono::milliseconds(TRANSMISSION_DELAY_MS)); // simulate radio transmission delay
        
        transmittedPackets_++;                // increment total transmitted packet count
        transmittedBytes_ += pdu.data.size(); // add PDU size to total byte counter
        
        auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - pdu.timestamp).count(); // calculate end-to-end latency from RLC enqueue time
        
        std::cout << "[MAC] Transmitted PDU ID=" << pdu.id 
                  << " latency=" << latency << "ms\n"; // log transmission result to console
        
        notifyObserver(); // inform registered observer about updated statistics
    }
}

void MACLayer::registerObserver(std::shared_ptr<IStatisticsObserver> observer) {
    observer_ = observer; // store observer for later statistics notifications
}

void MACLayer::notifyObserver() {
    if (observer_) { // only notify if an observer has been registered
        observer_->onStatisticsUpdate(getStatistics()); // push current statistics snapshot to observer
    }
}

} // namespace ue_simulator
