#ifndef LBM_CORE_HPP
#define LBM_CORE_HPP

#include <algorithm>
#include <cmath>
#include <omp.h>

#include "LatticeModels.hpp"

template <typename Lattice>
class Field {
public:
    int nx, ny, nz, total_cells;
    double *f_in, *f_out;

    Field(int lx, int ly, int lz) : nx(lx), ny(ly), nz(lz) {
        total_cells = nx * ny * nz;
        size_t bytes = sizeof(double) * Lattice::Q * total_cells;
        f_in  = new double[Lattice::Q * total_cells]();
        f_out = new double[Lattice::Q * total_cells]();

#pragma omp target enter data map(alloc: f_in[0:Lattice::Q*total_cells], f_out[0:Lattice::Q*total_cells])
    }

    ~Field() {
#pragma omp target exit data map(delete: f_in[0:Lattice::Q*total_cells], f_out[0:Lattice::Q*total_cells])
        delete[] f_in; delete[] f_out;
    }

    inline int idx(int q, int x, int y, int z) {
        return q * total_cells + (z * ny * nx + y * nx + x);
    }

    void swap() { std::swap(f_in, f_out); }
};

template <typename Lattice>
class LBMEngine {
public:
    static void step(Field<Lattice>& field, double tau) {
        int nx = field.nx;
        int ny = field.ny;
        int nz = field.nz;
        int total = field.total_cells;
        double *f_in = field.f_in;
        double *f_out = field.f_out;
        double inv_tau = 1.0 / tau;

#pragma omp target teams distribute parallel for collapse(3)
        for (int z = 0; z < nz; z++) {
            for (int y = 0; y < ny; y++) {
                for (int x = 0; x < nx; x++) {
                    int c_idx = z * ny * nx + y * nx + x;
                    
                    // 1. Moments
                    double rho = 0;
                    double ux = 0;
                    double uy = 0;
                    double uz = 0;
                    
                    for (int q = 0; q < Lattice::Q; q++) {
                        double f = f_in[q * total + c_idx];
                        rho += f;
                        ux += f * Lattice::e[q][0];
                        uy += f * Lattice::e[q][1];
                        uz += f * Lattice::e[q][2];
                    }
                    ux /= rho;
                    uy /= rho;
                    uz /= rho;

                    // 2. Collide & Stream
                    for (int q = 0; q < Lattice::Q; q++) {
                        double eu = (Lattice::e[q][0]*ux + Lattice::e[q][1]*uy + Lattice::e[q][2]*uz) / Lattice::Cs2;
                        double u2 = (ux*ux + uy*uy + uz*uz) / Lattice::Cs2;
                        double feq = Lattice::w[q] * rho * (1.0 + eu + 0.5*eu*eu - 0.5*u2);
                        
                        double f_post = f_in[q * total + c_idx] - inv_tau * (f_in[q * total + c_idx] - feq);

                        // Periodic Streaming logic
                        int ni = (x + Lattice::e[q][0] + nx) % nx;
                        int nj = (y + Lattice::e[q][1] + ny) % ny;
                        int nk = (z + Lattice::e[q][2] + nz) % nz;

                        // int ni = x + Lattice::e[q][0];
                        // int nj = y + Lattice::e[q][1];
                        // int nk = z + Lattice::e[q][2];
                        // // Only stream if within local bounds (including ghost cells)
                        // if (ni >= 0 && ni < nx && nj >= 0 && nj < ny && nk >= 0 && nk < nz) {
                        //     f_out[q * total + (nk * ny * nx + nj * nx + ni)] = f_post;
                        // }
 
                        f_out[q * total + (nk * ny * nx + nj * nx + ni)] = f_post;
                    }
                }
            }
        }
    }

    static void init_taylor_green(Field<Lattice>& field, int global_nx, int global_ny, int global_nz, int offset_x, int offset_y, int offset_z) {
        int nx = field.nx; int ny = field.ny; int nz = field.nz;
        int total = field.total_cells;
        double *f_host = field.f_in; // We fill on host then push to device

        const double u0 = 0.1; // Maximum velocity in lattice units
        const double rho0 = 1.0;

        using std::sin;
        using std::cos;

        for (int z = 0; z < nz; z++) {
            for (int y = 0; y < ny; y++) {
                for (int x = 0; x < nx; x++) {
                    // Global coordinates for the analytical trig functions
                    double gx = (double)(x + offset_x);
                    double gy = (double)(y + offset_y);
                    double gz = (double)(z + offset_z);
                    
                    // Taylor-Green analytical velocity field
                    double kx = 2.0 * M_PI / global_nx;
                    double ky = 2.0 * M_PI / global_ny;
                    double kz = 2.0 * M_PI / global_nz;
                    
                    double ux =  u0 * sin(kx * gx) * cos(ky * gy) * cos(kz * gz);
                    double uy = -u0 * cos(kx * gx) * sin(ky * gy) * cos(kz * gz);
                    double uz = 0.0;
                    double rho = rho0+u0*u0/Lattice::Cs2*rho0/16*(cos(2*kx*gx)+cos(2*ky*gy))*(cos(2*kz*gz)+2);

                    int c_idx = z * ny * nx + y * nx + x;
                    
                    // Initialize f_in to Equilibrium
                    for (int q = 0; q < Lattice::Q; q++) {
                        double eu = (Lattice::e[q][0]*ux + Lattice::e[q][1]*uy + Lattice::e[q][2]*uz) / Lattice::Cs2;
                        double u2 = (ux*ux + uy*uy + uz*uz) / Lattice::Cs2;
                        f_host[q * total + c_idx] = Lattice::w[q] * rho * (1.0 + eu + 0.5*eu*eu - 0.5*u2);
                    }
                }
            }
        }
        // Push initialized data to GPU
#pragma omp target update to(field.f_in[0:Lattice::Q*total])
    }
};
#endif
