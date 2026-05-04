#ifndef COMMON_TYPES_HPP
#define COMMON_TYPES_HPP

#include <vector>
#include <cstdint>
#include <chrono>
#include <memory>

namespace ue_simulator {

// 3GPP TS 38.321 - Priority levels for Logical Channels
enum class Priority : uint8_t {
    LOW = 1,
    MEDIUM = 2,
    HIGH = 3
};

// 3GPP TS 38.322 - RLC modes
enum class RlcMode {
    TM,  // Transparent Mode
    UM,  // Unacknowledged Mode
    AM   // Acknowledged Mode
};

// PDU (Protocol Data Unit) structure
struct PDU {
    uint32_t id;
    std::vector<uint8_t> data;
    Priority priority;
    std::chrono::steady_clock::time_point timestamp;
    
    PDU() : id(0), priority(Priority::MEDIUM) {
        timestamp = std::chrono::steady_clock::now();
    }
};

// Statistics structure
struct Statistics {
    uint64_t transmittedPackets{0};
    uint64_t transmittedBytes{0};
    uint64_t droppedPackets{0};
    uint32_t queueSize{0};
};

} // namespace ue_simulator

#endif // COMMON_TYPES_HPP
