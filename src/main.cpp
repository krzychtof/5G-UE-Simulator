#include "ue_simulator.hpp"
#include <iostream>
#include <thread>

int main() {
    try {
        ue_simulator::UESimulator simulator;
        
        simulator.start();
        simulator.generateTraffic(20);
        
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        simulator.stop();
        
        std::cout << "\n[SYSTEM] Simulation completed successfully!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
