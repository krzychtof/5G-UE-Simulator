#ifndef PDCP_LAYER_HPP
#define PDCP_LAYER_HPP

#include "ilayer.hpp"
#include <mutex>

namespace ue_simulator {

// 3GPP TS 38.323 - PDCP (Packet Data Convergence Protocol)
// Responsibilities: Header compression, ciphering, integrity protection
class PDCPLayer : public ILayer {
public:
    PDCPLayer();
    ~PDCPLayer() override = default;

    // ILayer interface implementation
    bool sendPDU(const PDU& pdu) override;
    bool receivePDU(PDU& pdu) override;
    Statistics getStatistics() const override;

    // PDCP specific operations
    std::vector<uint8_t> addHeader(const std::vector<uint8_t>& sdu);
    std::vector<uint8_t> removeHeader(const std::vector<uint8_t>& pdu);

private:
    mutable std::mutex mutex_;
    Statistics stats_;
    uint32_t sequenceNumber_;
    
    static constexpr uint8_t PDCP_HEADER = 0xAA;
    static constexpr uint8_t PDCP_TRAILER = 0xFF;
};

} // namespace ue_simulator

#endif // PDCP_LAYER_HPP
