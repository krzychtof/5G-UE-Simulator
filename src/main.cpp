#include "ue_simulator.hpp"
#include <iostream>
#include <thread>

int main() {
    try {
        // Create UE simulator instance
        ue_simulator::UESimulator simulator;
        
        // Start the simulator
        simulator.start();
        // Generate network traffic with 20 packets
        simulator.generateTraffic(20);
        
        // Wait 2 seconds for the simulation to process generated traffic
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // Stop the simulator and release resources
        simulator.stop();
        
        std::cout << "\n[SYSTEM] Simulation completed successfully!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
