#include "pdcp_layer.hpp"
#include <algorithm>

namespace ue_simulator {

PDCPLayer::PDCPLayer() : sequenceNumber_(0) {}

bool PDCPLayer::sendPDU(const PDU& pdu) {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_.transmittedPackets++;
    stats_.transmittedBytes += pdu.data.size();
    return true;
}

bool PDCPLayer::receivePDU(PDU& pdu) {
    std::lock_guard<std::mutex> lock(mutex_);
    (void)pdu; // Unused in prototype
    return false;
}


Statistics PDCPLayer::getStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

std::vector<uint8_t> PDCPLayer::addHeader(const std::vector<uint8_t>& sdu) {
    std::vector<uint8_t> pdu;
    pdu.reserve(sdu.size() + 2);
    
    // 3GPP TS 38.323 - PDCP header
    pdu.push_back(PDCP_HEADER);
    pdu.insert(pdu.end(), sdu.begin(), sdu.end());
    pdu.push_back(PDCP_TRAILER);
    
    return pdu;
}

std::vector<uint8_t> PDCPLayer::removeHeader(const std::vector<uint8_t>& pdu) {
    if (pdu.size() < 2) {
        return {};
    }
    return std::vector<uint8_t>(pdu.begin() + 1, pdu.end() - 1);
}

} // namespace ue_simulator
