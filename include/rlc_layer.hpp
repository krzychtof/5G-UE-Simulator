#ifndef RLC_LAYER_HPP
#define RLC_LAYER_HPP

#include "ilayer.hpp"
#include <queue>
#include <mutex>
#include <condition_variable>

namespace ue_simulator {

// 3GPP TS 38.322 - RLC (Radio Link Control)
// Responsibilities: Segmentation, ARQ, duplicate detection
class RLCLayer : public ILayer {
public:
    // Constructor with RLC mode parameter, defaults to UM (Unacknowledged Mode)
    explicit RLCLayer(RlcMode mode = RlcMode::UM);
    // Virtual destructor with default implementation
    ~RLCLayer() override = default;

    // ILayer interface implementation
    // Sends a PDU (Protocol Data Unit) - returns true on success
    bool sendPDU(const PDU& pdu) override;
    // Receives a PDU by reference - returns true on success
    bool receivePDU(PDU& pdu) override;
    // Returns RLC layer statistics - const, does not modify the object
    Statistics getStatistics() const override;

    // RLC specific operations
    // Segments an SDU (Service Data Unit) into smaller pieces with the given priority
    void segmentSDU(const std::vector<uint8_t>& sdu, Priority priority);
    // Returns the current size of the transmit queue
    size_t getQueueSize() const;
    // Returns true if the transmit queue is empty
    bool isQueueEmpty() const;

private:
    mutable std::mutex mutex_;       // Mutex for synchronizing shared data access across threads
    std::condition_variable cv_;     // Condition variable for inter-thread signalling
    std::queue<PDU> txQueue_;        // FIFO queue holding PDUs pending transmission
    Statistics stats_;               // RLC layer statistics
    RlcMode mode_;                   // RLC operating mode (AM, UM, TM)
    uint32_t pduCounter_;            // PDU counter for packet numbering
    
    static constexpr size_t MAX_QUEUE_SIZE = 1000;  // Maximum queue capacity
};

} // namespace ue_simulator

#endif // RLC_LAYER_HPP
