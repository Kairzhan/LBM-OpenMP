#ifndef LATTICE_MODELS_H
#define LATTICE_MODELS_H

namespace LBM {
    // D3Q19: Standard for 3D Navier-Stokes
    struct D3Q19 {
        static constexpr int Q = 19;
        static constexpr double Cs2 = 1.0 / 3.0;
        
        // Velocity vectors (ex, ey, ez)
        static constexpr int e[19][3] = {
            {0,0,0}, {1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1},
            {1,1,0}, {-1,-1,0}, {1,-1,0}, {-1,1,0}, {1,0,1}, {-1,0,-1}, {1,0,-1},
            {-1,0,1}, {0,1,1}, {0,-1,-1}, {0,1,-1}, {0,-1,1}
        };

        // Weights
        static constexpr double w[19] = {
            1.0/3.0, 1.0/18.0, 1.0/18.0, 1.0/18.0, 1.0/18.0, 1.0/18.0, 1.0/18.0,
            1.0/36.0, 1.0/36.0, 1.0/36.0, 1.0/36.0, 1.0/36.0, 1.0/36.0, 1.0/36.0,
            1.0/36.0, 1.0/36.0, 1.0/36.0, 1.0/36.0, 1.0/36.0
        };

        // Opposite indices for bounce-back (0 is self)
        static constexpr int opp[19] = {0, 2, 1, 4, 3, 6, 5, 8, 7, 10, 9, 12, 11, 14, 13, 16, 15, 18, 17};
    };

    // D3Q7: Efficient for Passive Scalar / Temperature
    struct D3Q7 {
        static constexpr int Q = 7;
        static constexpr double Cs2 = 1.0 / 4.0; // Note: Cs^2 varies by model

        static constexpr int e[7][3] = {
            {0,0,0}, {1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1}
        };

        static constexpr double w[7] = {
            1.0/4.0, 1.0/8.0, 1.0/8.0, 1.0/8.0, 1.0/8.0, 1.0/8.0, 1.0/8.0
        };

        static constexpr int opp[7] = {0, 2, 1, 4, 3, 6, 5};
    };

}
#endif
