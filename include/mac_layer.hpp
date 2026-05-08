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
    // Explicit constructor accepting a pointer to the RLC layer
    explicit MACLayer(std::shared_ptr<RLCLayer> rlcLayer);
    // Virtual destructor with default implementation
    ~MACLayer() override = default;

    // ILayer interface implementation
    // Sends a Protocol Data Unit (PDU), returns bool indicating success
    bool sendPDU(const PDU& pdu) override;
    // Receives a PDU by reference, returns bool indicating success
    bool receivePDU(PDU& pdu) override;
    // Returns MAC layer statistics
    Statistics getStatistics() const override;

    // MAC specific operations
    // Schedules the next data transmission
    void scheduleTransmission();
    // Registers a statistics observer
    void registerObserver(std::shared_ptr<IStatisticsObserver> observer);

private:
    // Pointer to the RLC (Radio Link Control) layer
    std::shared_ptr<RLCLayer> rlcLayer_;
    // Atomic transmitted packet counter (thread-safe)
    std::atomic<uint64_t> transmittedPackets_;
    // Atomic transmitted byte counter (thread-safe)
    std::atomic<uint64_t> transmittedBytes_;
    // Pointer to the statistics observer
    std::shared_ptr<IStatisticsObserver> observer_;
    
    // Transmission delay constant in milliseconds (10ms)
    static constexpr int TRANSMISSION_DELAY_MS = 10;
    
    // Private method to notify the observer of changes
    void notifyObserver();
};

} // namespace ue_simulator

#endif // MAC_LAYER_HPP
