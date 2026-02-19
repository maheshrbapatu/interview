// C = A * B
// A: (M x K), B: (K x N), C: (M x N) row-major
// Compile with: nvcc -O3 matmul.cu

#include <cuda_runtime.h>
#include <cstdio>

#ifndef TILE
#define TILE 16
#endif

__global__ void matmul_tiled(const float* __restrict__ A,
                             const float* __restrict__ B,
                             float* __restrict__ C,
                             int M, int N, int K) {
    __shared__ float As[TILE][TILE];
    __shared__ float Bs[TILE][TILE];

    int row = blockIdx.y * TILE + threadIdx.y;
    int col = blockIdx.x * TILE + threadIdx.x;

    float acc = 0.0f;

    // Iterate over K dimension in TILE-sized chunks
    for (int t = 0; t < (K + TILE - 1) / TILE; ++t) {
        int aCol = t * TILE + threadIdx.x;   // column in A
        int bRow = t * TILE + threadIdx.y;   // row in B

        // Load A tile element (or 0 if out of bounds)
        As[threadIdx.y][threadIdx.x] =
            (row < M && aCol < K) ? A[row * K + aCol] : 0.0f;

        // Load B tile element (or 0 if out of bounds)
        Bs[threadIdx.y][threadIdx.x] =
            (bRow < K && col < N) ? B[bRow * N + col] : 0.0f;

        __syncthreads();

        // Multiply the two shared tiles
        #pragma unroll
        for (int k = 0; k < TILE; ++k) {
            acc += As[threadIdx.y][k] * Bs[k][threadIdx.x];
        }

        __syncthreads();
    }

    if (row < M && col < N) {
        C[row * N + col] = acc;
    }
}

void launch(const float* A, const float* B, float* C, int M, int N, int K) {
    dim3 block(TILE, TILE);
    dim3 grid((N + TILE - 1) / TILE, (M + TILE - 1) / TILE);
    matmul_tiled<<<grid, block>>>(A, B, C, M, N, K);
}
