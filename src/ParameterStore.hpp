#ifndef PARAMETER_STORE_HPP
#define PARAMETER_STORE_HPP

#include <string>
#include <vector>
#include <array>
#include <iostream>

#include "ParameterReader.hpp"

class ParameterStore {
public:
    ParameterStore(const ParameterStore&) = delete;
    void operator=(const ParameterStore&) = delete;

    static ParameterStore& getInstance() {
        static ParameterStore instance;
        return instance;
    }

    std::array<int, 3> grid{0, 0, 0};
    unsigned nx, ny, nz;
    unsigned lx, ly, lz;
    
    std::array<std::string, 6> boundary_types;

    const double default_viscosity=0.166;
    const double default_RT=1./3.;
    double viscosity = default_viscosity;
    double tau=default_viscosity/default_RT+0.5;
    std::array<double, 3> gravity{0.0, 0.0, 0.0};
    std::array<double, 3> ambient_force{0.0, 0.0, 0.0};

    unsigned istep0 = 1;
    unsigned nsteps = 100;
    unsigned nrestart = 0;
    bool restart = false;

    unsigned noutput3 = 1000;
    unsigned noutput2 = 1000;
    unsigned ndump = 1000;

    unsigned ndiag = 100;
    unsigned nstat = 200;

    bool perturb;

    unsigned gpus_per_thread;
    
    void updateFrom(ParameterReader& reader) {
        auto temp_grid = reader.getList<int>("grid");
        std::copy_n(temp_grid.begin(), 3, grid.begin());
        
        nx=grid[0];
        ny=grid[1];
        nz=grid[2];

        auto temp_boundary_types = reader.getListString("boundary_types");
        std::copy_n(temp_boundary_types.begin(), 6, boundary_types.begin());
        
        viscosity=reader.get<float>("viscosity");
        tau=3.*viscosity+0.5;
        
        auto temp_gravity = reader.getList<float>("gravity");
        std::copy_n(temp_gravity.begin(), 3, gravity.begin());
        auto temp_ambient_force = reader.getList<float>("ambient_force");
        std::copy_n(temp_ambient_force.begin(), 3, ambient_force.begin());

        istep0 = reader.get<int>("istep0", 1);
        nsteps = reader.get<int>("nsteps", 1000);
        restart = reader.getBool("restart", true);
        nrestart = reader.get<int>("nrestart", 1000);
        
        noutput3 = reader.get<int>("noutput3", 1000);
        noutput2 = reader.get<int>("noutput2", 1000);
        ndump = reader.get<int>("ndump", 1000);
        
        ndiag = reader.get<int>("ndiag", 1000);
        nstat = reader.get<int>("nstat", 1000);

        perturb = reader.getBool("perturb", true);
    }

    void display() const {
        std::cout << "\n================ SIMULATION PARAMETERS ================" << std::endl;
        
        std::cout << "[Domain & Grid]\n";
        std::cout << "  Grid:           [" << grid[0] << ", " << grid[1] << ", " << grid[2] << "]\n";
        std::cout << "  Resolution:     " << nx << "x" << ny << "x" << nz << "\n";
        std::cout << "  Local Size:  " << lx << "x" << ly << "x" << lz << "\n";
        std::cout << "  Boundaries:     " << boundary_types[0] << ", " << boundary_types[1] << ", " << boundary_types[2] << ", " \
                  << boundary_types[3] << ", " << boundary_types[4] << ", " << boundary_types[5] << "\n\n";

        std::cout << "[Physics]\n";
        std::cout << "  Viscosity:      " << viscosity << "\n";
        std::cout << "  Gravity:        [" << gravity[0] << ", " << gravity[1] << ", " << gravity[2] << "]\n";
        std::cout << "  Ambient Force:  [" << ambient_force[0] << ", " << ambient_force[1] << ", " << ambient_force[2] << "]\n\n";

        std::cout << "[Control]\n";
        std::cout << "  Steps:          " << istep0 << " to " << nsteps << " (Restart: " << (restart ? "Yes" : "No") << ")\n";
        std::cout << "  Perturb:        " << (perturb ? "Enabled" : "Disabled") << "\n\n";

        std::cout << "[Output Intervals]\n";
        std::cout << "  3D Out: " << noutput3 << " | 2D Out: " << noutput2 << " | Dump: " << ndump << "\n";
        std::cout << "  Diag:   " << ndiag << " | Stat:   " << nstat << "\n\n";

        std::cout << "[Hardware]\n";
        std::cout << "  GPUs/Thread:    " << gpus_per_thread << "\n";
        std::cout << "=======================================================\n" << std::endl;
    }
    
private:
    ParameterStore() {} 
};

#endif
