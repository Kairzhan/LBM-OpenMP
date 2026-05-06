#ifndef MACROSCOPIC_HPP
#define MACROSCOPIC_HPP

#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <arpa/inet.h>

#include "LBM_Core.hpp"

struct MacroscopicField {
    int nx, ny, nz;
    std::vector<double> rho;
    std::vector<double> ux, uy, uz;
    std::vector<double> temp; // For the D3Q7 field

    MacroscopicField(int lx, int ly, int lz) : nx(lx), ny(ly), nz(lz) {
        int size = nx * ny * nz;
        rho.resize(size);
        ux.resize(size);
        uy.resize(size);
        uz.resize(size);
        temp.resize(size);
    }

    // Function to pull data from GPU and calculate rho/u
    template <typename Lattice>
    void update(Field<Lattice>& fluid, Field<LBM::D3Q7>& thermal) {
        int total = fluid.total_cells;
        
        // Ensure data is current on Host
#pragma omp target update from(fluid.f_in[0:Lattice::Q*total], thermal.f_in[0:7*total])

        for (int z = 0; z < nz; z++) {
            for (int y = 0; y < ny; y++) {
                for (int x = 0; x < nx; x++) {
                    int local_idx = z * ny * nx + y * nx + x;
                    
                    // Fluid moments (D3Q19)
                    double r = 0, vx = 0, vy = 0, vz = 0;
                    for (int q = 0; q < Lattice::Q; q++) {
                        double f = fluid.f_in[q * total + local_idx];
                        r += f;
                        vx += f * Lattice::e[q][0];
                        vy += f * Lattice::e[q][1];
                        vz += f * Lattice::e[q][2];
                    }
                    rho[local_idx] = r;
                    ux[local_idx] = vx / r;
                    uy[local_idx] = vy / r;
                    uz[local_idx] = vz / r;

                    // Thermal moment (D3Q7)
                    double t_val = 0;
                    for (int q = 0; q < 7; q++) {
                        t_val += thermal.f_in[q * total + local_idx];
                    }
                    temp[local_idx] = t_val;
                }
            }
        }
    }

    void write_tecplot(int rank, int timestep) {
        std::string filename = "out_r" + std::to_string(rank) + "_t" + std::to_string(timestep) + ".dat";
        std::ofstream fout(filename);

        fout << "TITLE = \"LBM Field Data\"\n";
        fout << "VARIABLES = \"X\", \"Y\", \"Z\", \"RHO\", \"U\", \"V\", \"W\", \"TEMPERATURE\"\n";
        fout << "ZONE I=" << nx << ", J=" << ny << ", K=" << nz << ", F=POINT\n";

        for (int k = 0; k < nz; k++) {
            for (int j = 0; j < ny; j++) {
                for (int i = 0; i < nx; i++) {
                    int idx = k * ny * nx + j * nx + i;
                    fout << i << " " << j << " " << k << " "
                         << std::scientific << std::setprecision(6)
                         << rho[idx] << " "
                         << ux[idx] << " " << uy[idx] << " " << uz[idx] << " "
                         << temp[idx] << "\n";
                }
            }
        }
        fout.close();
    }

    // Helper to swap endianness to Big-Endian (VTK standard)
    float swap_float(float val) {
        union { float f; uint32_t i; } u;
        u.f = val;
        u.i = htonl(u.i); // host-to-network-long is Big-Endian
        return u.f;
    }

    void write_binary_vtk(int rank, int timestep, int off_x, int off_y, int off_z) {
        std::stringstream ss;
        ss << std::setw(4) << std::setfill('0') << rank;
        ss << "_";
        ss << std::setw(8) << std::setfill('0') << timestep;
        
        // std::string filename = "out_r" + std::to_string(rank) + "_" + ss.str() + ".vtk";
        std::string filename = "out_r" + ss.str() + ".vtk";
        std::ofstream fout(filename, std::ios::out | std::ios::binary);

        // 1. Header (ASCII part)
        fout << "# vtk DataFile Version 3.0\n";
        fout << "LBM Rank " << rank << " Time " << timestep << "\n";
        fout << "BINARY\n";
        fout << "DATASET STRUCTURED_POINTS\n";
        fout << "DIMENSIONS " << nx << " " << ny << " " << nz << "\n";
        fout << "ORIGIN " << off_x << " " << off_y << " " << off_z << "\n";
        fout << "SPACING 1 1 1\n";
        fout << "POINT_DATA " << nx * ny * nz << "\n";

        // 2. Scalar: Density
        fout << "SCALARS Density float 1\n";
        fout << "LOOKUP_TABLE default\n";
        for (double val : rho) {
            float f = swap_float((float)val);
            fout.write(reinterpret_cast<char*>(&f), sizeof(float));
        }
        fout << "\n";

        // 3. Vector: Velocity
        fout << "VECTORS Velocity float\n";
        for (int i = 0; i < nx * ny * nz; ++i) {
            float v[3];
            v[0] = swap_float((float)ux[i]);
            v[1] = swap_float((float)uy[i]);
            v[2] = swap_float((float)uz[i]);
            fout.write(reinterpret_cast<char*>(v), 3 * sizeof(float));
        }
        fout << "\n";

        // 4. Scalar: Temperature
        fout << "SCALARS Temperature float 1\n";
        fout << "LOOKUP_TABLE default\n";
        for (double val : temp) {
            float f = swap_float((float)val);
            fout.write(reinterpret_cast<char*>(&f), sizeof(float));
        }
    
        fout.close();
    }
};

#endif
