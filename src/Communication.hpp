#ifndef COMMUNICATION_HPP
#define COMMUNICATION_HPP

#include <mpi.h>
#include <vector>
#include "LBM_Core.hpp"

enum Side { LEFT = 0, RIGHT = 1, BOTTOM = 2, TOP = 3, BACK = 4, FRONT = 5 };

struct MPIContext {
    int rank, size;
    int dims[3] = {0, 0, 0};
    int coords[3];
    int neighbors[6]; // LEFT, RIGHT, BOTTOM, TOP, BACK, FRONT
    MPI_Comm cart_comm;

    void init(int total_ranks) {
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        
        // Create 3D Cartesian Topology
        MPI_Dims_create(size, 3, dims);
        int periods[3] = {1, 1, 1}; // Periodic boundaries
        MPI_Cart_create(MPI_COMM_WORLD, 3, dims, periods, 0, &cart_comm);
        MPI_Cart_coords(cart_comm, rank, 3, coords);

        // Find neighbors
        MPI_Cart_shift(cart_comm, 0, 1, &neighbors[LEFT], &neighbors[RIGHT]);
        MPI_Cart_shift(cart_comm, 1, 1, &neighbors[BOTTOM], &neighbors[TOP]);
        MPI_Cart_shift(cart_comm, 2, 1, &neighbors[BACK], &neighbors[FRONT]);
    }
};

template <typename Lattice>
void exchange_halos(Field<Lattice>& field, MPIContext& ctx) {
    int nx = field.nx;
    int ny = field.ny;
    int nz = field.nz;
    int Q = Lattice::Q;

    // Example for X-direction (Left/Right)
    // In a real solver, you repeat this for Y and Z directions
    int face_size = Q * ny * nz;
    std::vector<double> send_buf(face_size), recv_buf(face_size);

    // 1. Pack Right boundary to send to RIGHT neighbor
    // (This should be an OpenMP GPU kernel in production)
#pragma omp target update from(field.f_out[0:Q*field.total_cells]) 
    
    // MPI SendRecv (X-axis)
    MPI_Status status;
    MPI_Sendrecv(&field.f_out[field.idx(0, nx-2, 0, 0)], 1, MPI_DOUBLE, ctx.neighbors[RIGHT], 0,
                 &field.f_out[field.idx(0, 0, 0, 0)], 1, MPI_DOUBLE, ctx.neighbors[LEFT], 0, 
                 ctx.cart_comm, &status);

#pragma omp target update to(field.f_out[0:Q*field.total_cells])
}

#endif
