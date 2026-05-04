#ifndef ILAYER_HPP
#define ILAYER_HPP

#include "common_types.hpp"

namespace ue_simulator {

// Interface for protocol layers (Dependency Inversion Principle)
class ILayer {
public:
    virtual ~ILayer() = default;
    virtual bool sendPDU(const PDU& pdu) = 0;
    virtual bool receivePDU(PDU& pdu) = 0;
    virtual Statistics getStatistics() const = 0;
};

// Observer pattern for statistics monitoring
class IStatisticsObserver {
public:
    virtual ~IStatisticsObserver() = default;
    virtual void onStatisticsUpdate(const Statistics& stats) = 0;
};

} // namespace ue_simulator

#endif // ILAYER_HPP
