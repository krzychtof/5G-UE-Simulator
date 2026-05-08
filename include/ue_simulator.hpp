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
    // Constructor
    UESimulator();
    // Destructor
    ~UESimulator();

    // Simulator control
    // Starts the simulator
    void start();
    // Stops the simulator
    void stop();
    // Generates network traffic, default 20 packets
    void generateTraffic(size_t numPackets = 20);

    // IStatisticsObserver implementation
    // Called on statistics update - overrides base class method
    void onStatisticsUpdate(const Statistics& stats) override;

private:
    // Protocol layers (Dependency Injection)
    // Pointer to the PDCP (Packet Data Convergence Protocol) layer
    std::shared_ptr<PDCPLayer> pdcpLayer_;
    // Pointer to the RLC (Radio Link Control) layer
    std::shared_ptr<RLCLayer> rlcLayer_;
    // Pointer to the MAC (Medium Access Control) layer
    std::shared_ptr<MACLayer> macLayer_;

    // Threading
    // Atomic boolean flag controlling the simulator running state
    std::atomic<bool> running_;
    // Thread responsible for data transmission
    std::thread txThread_;
    // Thread responsible for collecting and displaying statistics
    std::thread statsThread_;

    // Private methods
    // Main transmission loop running in a dedicated thread
    void transmissionLoop();
    // Main statistics loop running in a dedicated thread
    void statisticsLoop();
    // Prints the startup banner (read-only)
    void printBanner() const;
};

} // namespace ue_simulator

#endif // UE_SIMULATOR_HPP
