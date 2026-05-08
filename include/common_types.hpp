#ifndef COMMON_TYPES_HPP
#define COMMON_TYPES_HPP

#include <vector>
#include <cstdint>
#include <chrono>
#include <memory>

namespace ue_simulator {

// 3GPP TS 38.321 - Priority levels for Logical Channels
// Priority enum class with uint8_t underlying type - represents logical channel priority levels per 3GPP standard
enum class Priority : uint8_t {
    LOW = 1,     // Low priority - value 1
    MEDIUM = 2,  // Medium priority - value 2
    HIGH = 3     // High priority - value 3
};

// 3GPP TS 38.322 - RLC modes
// RlcMode enumeration - represents RLC layer operating modes per 3GPP standard
enum class RlcMode {
    TM,  // Transparent Mode
    UM,  // Unacknowledged Mode
    AM   // Acknowledged Mode
};

// PDU (Protocol Data Unit) structure
// Represents a single protocol data unit passed between layers
struct PDU {
    uint32_t id;                                        // PDU identifier - unsigned 32-bit integer
    std::vector<uint8_t> data;                          // PDU payload - byte vector
    Priority priority;                                  // PDU priority - uses Priority enum
    std::chrono::steady_clock::time_point timestamp;    // PDU timestamp - monotonic clock time point
    
    // Default constructor - initializes id to 0, priority to MEDIUM, timestamp to now
    PDU() : id(0), priority(Priority::MEDIUM) {
        timestamp = std::chrono::steady_clock::now();
    }
};

// Statistics structure
// Holds transmission statistics collected by protocol layers
struct Statistics {
    uint64_t transmittedPackets{0};  // Number of transmitted packets
    uint64_t transmittedBytes{0};    // Number of transmitted bytes
    uint64_t droppedPackets{0};      // Number of dropped packets
    uint32_t queueSize{0};           // Current queue size
};

} // namespace ue_simulator

#endif // COMMON_TYPES_HPP
