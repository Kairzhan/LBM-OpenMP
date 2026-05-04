#include "ParameterReader.hpp"
#include <iostream>

int main() {
    ParameterReader config;

    if (config.load("config.txt")) {
        // Demonstrate case insensitivity and type casting
        int nx = config.get<int>("NX", 60);
        float nsteps = config.get<int>("nsteps", 1000);
        bool restart = config.getBool("restart", true);

        // 1. Get Integers
        std::vector<int> grid = config.getList<int>("grid");
    
        // 2. Get Floats
        std::vector<float> gravity = config.getList<float>("gravity");
    
        // 3. Get Strings (using the specialized string helper)
        std::vector<std::string> boundary_types = config.getListString("boundary_types");

        for (auto bt: boundary_types) {
            std::cout << bt << " ";
        }
        std::cout << std::endl;

        std::cout << "NX:" << nx << std::endl;
        std::cout << "Nsteps:" << nsteps << std::endl;
        
        
        std::cout << "Grid: " << grid[0] << " " << grid[1] << " " << grid[2]<< "\n"
                  << "Gravity: " << gravity[0] << "\n"
                  << "Restart: " << (restart ? "Enabled" : "Disabled") << std::endl;
    } else {
        std::cerr << "Failed to load config file." << std::endl;
    }

    return 0;
}
