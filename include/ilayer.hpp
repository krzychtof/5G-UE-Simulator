#ifndef ILAYER_HPP
#define ILAYER_HPP

#include "common_types.hpp"

namespace ue_simulator {

// Interface for protocol layers (Dependency Inversion Principle)
class ILayer {
public:
    // Virtual destructor ensuring correct destruction of derived objects
    virtual ~ILayer() = default;
    // Pure virtual method for sending a Protocol Data Unit (PDU)
    virtual bool sendPDU(const PDU& pdu) = 0;
    // Pure virtual method for receiving a Protocol Data Unit (PDU)
    virtual bool receivePDU(PDU& pdu) = 0;
    // Pure virtual method for retrieving layer statistics
    virtual Statistics getStatistics() const = 0;
};

// Observer pattern for statistics monitoring
class IStatisticsObserver {
public:
    // Virtual destructor ensuring correct destruction of derived objects
    virtual ~IStatisticsObserver() = default;
    // Pure virtual method called on statistics update
    virtual void onStatisticsUpdate(const Statistics& stats) = 0;
};

} // namespace ue_simulator

#endif // ILAYER_HPP
