#include "pdcp_layer.hpp" // Include PDCP layer header with class declaration
#include <algorithm>          // Include standard algorithms (unused directly, kept for potential extensions)

namespace ue_simulator {

PDCPLayer::PDCPLayer() : sequenceNumber_(0) {} // Initialize sequence number counter to 0

bool PDCPLayer::sendPDU(const PDU& pdu) {
    std::lock_guard<std::mutex> lock(mutex_); // Lock mutex to protect shared statistics
    stats_.transmittedPackets++;              // Increment transmitted packet counter
    stats_.transmittedBytes += pdu.data.size(); // Add PDU size to total transmitted bytes
    return true; // Always succeeds in this prototype
}

bool PDCPLayer::receivePDU(PDU& pdu) {
    std::lock_guard<std::mutex> lock(mutex_); // Lock mutex for thread safety
    (void)pdu; // Suppress unused parameter warning - not implemented in prototype
    return false; // Receiving not implemented in this prototype
}

Statistics PDCPLayer::getStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_); // Lock mutex before reading shared statistics
    return stats_; // Return copy of current statistics
}

std::vector<uint8_t> PDCPLayer::addHeader(const std::vector<uint8_t>& sdu) {
    std::vector<uint8_t> pdu;
    pdu.reserve(sdu.size() + 2); // Pre-allocate memory for SDU + 1 header byte + 1 trailer byte

    // 3GPP TS 38.323 - PDCP header
    pdu.push_back(PDCP_HEADER);                  // Prepend PDCP header byte (0xAA)
    pdu.insert(pdu.end(), sdu.begin(), sdu.end()); // Copy original SDU payload into PDU
    pdu.push_back(PDCP_TRAILER);                 // Append PDCP trailer byte (0xFF)

    return pdu; // Return fully wrapped PDU
}

std::vector<uint8_t> PDCPLayer::removeHeader(const std::vector<uint8_t>& pdu) {
    if (pdu.size() < 2) { // Check if PDU is too small to contain header and trailer
        return {};        // Return empty vector if malformed
    }
    return std::vector<uint8_t>(pdu.begin() + 1, pdu.end() - 1); // Strip first (header) and last (trailer) bytes
}

} // namespace ue_simulator
