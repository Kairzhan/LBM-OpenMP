#include <iostream>
#include <mpi.h>

#include "ParameterReader.hpp"
#include "ParameterStore.hpp"
#include "LBM_Core.hpp"
#include "Communication.hpp"
#include "Macroscopic.hpp"

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    
    MPIContext ctx;
    ctx.init(0); // Initialize 3D topology

    // Read simulation parameters first
    ParameterReader reader;    
    if (reader.load("config.ini")) {
        ParameterStore::getInstance().updateFrom(reader);
    } else {
        std::cerr << "ERROR: config.ini not found !\n";
    }
    ParameterStore::getInstance().display();

    // Domain sizes (simplified: same for all ranks)
    const int NX = ParameterStore::getInstance().nx;
    const int NY = ParameterStore::getInstance().ny;
    const int NZ = ParameterStore::getInstance().nz;

    // Initialization: One fluid lattice, one thermal lattice
    Field<LBM::D3Q19> fluid(NX, NY, NZ);
    Field<LBM::D3Q7>  thermal(NX, NY, NZ);

    LBMEngine<LBM::D3Q19>::init_taylor_green(fluid, NX, NY, NZ, 0, 0, 0);
    
    MacroscopicField macro(NX, NY, NZ);
    
    if (ctx.rank == 0) std::cout << "Starting LBM Hybrid MPI/OpenMP Simulation..." << std::endl;

    MPI_Barrier(ctx.cart_comm);
    double start_time = MPI_Wtime();
    
    for (int istep = ParameterStore::getInstance().istep0; istep <= ParameterStore::getInstance().nsteps; istep++) {
        // Compute Physics on GPU
        LBMEngine<LBM::D3Q19>::step(fluid, ParameterStore::getInstance().tau);
        //        LBMEngine<LBM::D3Q7>::step(thermal, 1.2);

        // MPI Halo Exchange would happen here:
        // 1. Pack boundary f_out into buffers
        // 2. MPI_Sendrecv
        // 3. Unpack into f_out ghost cells
        //exchange_halos(fluid, ctx);
        
        fluid.swap();
        //        thermal.swap();

        if (istep % 200 == 0) {
            macro.update(fluid, thermal);
            //macro.write_tecplot(ctx.rank, istep);
            macro.write_binary_vtk(ctx.rank, istep, 0, 0, 0);
            if (ctx.rank == 0) std::cout << "Saved data at t=" << istep << std::endl;
        }
        
        if (ctx.rank == 0 && istep % 100 == 0) std::cout << "Step " << istep << " complete." << std::endl;
    }

    MPI_Barrier(ctx.cart_comm);
    double end_time = MPI_Wtime();

    double total_elapsed = end_time - start_time;

    if (ctx.rank == 0) {
        auto num_steps= ParameterStore::getInstance().nsteps;
        double time_per_step = total_elapsed / num_steps;
        
        // LBM Performance metric: Millions of Lattice Site Updates Per Second (MLUPS)
        double total_sites = (double)ctx.size * NX * NY * NZ;
        double mlups = (total_sites * num_steps) / (total_elapsed * 1e6);

        std::cout << "------------------------------------------" << std::endl;
        std::cout << "Simulation Finished in: " << total_elapsed << " seconds" << std::endl;
        std::cout << "Average time per step: " << time_per_step << " seconds" << std::endl;
        std::cout << "Performance:           " << mlups << " MLUPS" << std::endl;
        std::cout << "------------------------------------------" << std::endl;
    }
         
    MPI_Finalize();
    return 0;
}
