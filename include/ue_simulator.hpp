#ifndef UE_SIMULATOR_HPP
#define UE_SIMULATOR_HPP

#include "pdcp_layer.hpp"
#include "rlc_layer.hpp"
#include "mac_layer.hpp"
#include <thread>
#include <atomic>
#include <memory>

namespace ue_simulator {

// Facade pattern - simplifies interaction with protocol stack
class UESimulator : public IStatisticsObserver {
public:
    UESimulator();
    ~UESimulator();

    // Simulator control
    void start();
    void stop();
    void generateTraffic(size_t numPackets = 20);

    // IStatisticsObserver implementation
    void onStatisticsUpdate(const Statistics& stats) override;

private:
    // Protocol layers (Dependency Injection)
    std::shared_ptr<PDCPLayer> pdcpLayer_;
    std::shared_ptr<RLCLayer> rlcLayer_;
    std::shared_ptr<MACLayer> macLayer_;

    // Threading
    std::atomic<bool> running_;
    std::thread txThread_;
    std::thread statsThread_;

    // Private methods
    void transmissionLoop();
    void statisticsLoop();
    void printBanner() const;
};

} // namespace ue_simulator

#endif // UE_SIMULATOR_HPP
