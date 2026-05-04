#ifndef MAC_LAYER_HPP
#define MAC_LAYER_HPP

#include "ilayer.hpp"
#include "rlc_layer.hpp"
#include <atomic>
#include <memory>

namespace ue_simulator {

// 3GPP TS 38.321 - MAC (Medium Access Control)
// Responsibilities: Scheduling, multiplexing, HARQ, random access
class MACLayer : public ILayer {
public:
    explicit MACLayer(std::shared_ptr<RLCLayer> rlcLayer);
    ~MACLayer() override = default;

    // ILayer interface implementation
    bool sendPDU(const PDU& pdu) override;
    bool receivePDU(PDU& pdu) override;
    Statistics getStatistics() const override;

    // MAC specific operations
    void scheduleTransmission();
    void registerObserver(std::shared_ptr<IStatisticsObserver> observer);

private:
    std::shared_ptr<RLCLayer> rlcLayer_;
    std::atomic<uint64_t> transmittedPackets_;
    std::atomic<uint64_t> transmittedBytes_;
    std::shared_ptr<IStatisticsObserver> observer_;
    
    static constexpr int TRANSMISSION_DELAY_MS = 10;
    
    void notifyObserver();
};

} // namespace ue_simulator

#endif // MAC_LAYER_HPP
