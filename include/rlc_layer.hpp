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
    explicit RLCLayer(RlcMode mode = RlcMode::UM);
    ~RLCLayer() override = default;

    // ILayer interface implementation
    bool sendPDU(const PDU& pdu) override;
    bool receivePDU(PDU& pdu) override;
    Statistics getStatistics() const override;

    // RLC specific operations
    void segmentSDU(const std::vector<uint8_t>& sdu, Priority priority);
    size_t getQueueSize() const;
    bool isQueueEmpty() const;

private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<PDU> txQueue_;
    Statistics stats_;
    RlcMode mode_;
    uint32_t pduCounter_;
    
    static constexpr size_t MAX_QUEUE_SIZE = 1000;
};

} // namespace ue_simulator

#endif // RLC_LAYER_HPP
