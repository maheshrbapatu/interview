#include <bits/stdc++.h>
#include <cuda_runtime.h>
using namespace std;

constexpr int TILE = 16;

__global__ void transposeTiled(const int* in, int* out, int m, int n) {
    __shared__ int tile[TILE][TILE + 1]; // +1 padding avoids bank conflicts

    int in_col = blockIdx.x * TILE + threadIdx.x;
    int in_row = blockIdx.y * TILE + threadIdx.y;

    if (in_row < m && in_col < n) {
        tile[threadIdx.y][threadIdx.x] = in[in_row * n + in_col];
    }
    __syncthreads();

    int out_col = blockIdx.y * TILE + threadIdx.x;
    int out_row = blockIdx.x * TILE + threadIdx.y;

    if (out_row < n && out_col < m) {
        out[out_row * m + out_col] = tile[threadIdx.x][threadIdx.y];
    }
}

int main() {
    int m = 1000, n = 1000;
    int bytes = m * n * (int)sizeof(int);

    static int h_in[1000][1000];
    static int h_out[1000][1000];

    int x = 0;
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            h_in[i][j] = x++;

    int *d_in = nullptr, *d_out = nullptr;
    cudaMalloc(&d_in, bytes);
    cudaMalloc(&d_out, bytes);

    cudaMemcpy(d_in, h_in, bytes, cudaMemcpyHostToDevice);

    dim3 block(TILE, TILE);
    dim3 grid((n + TILE - 1) / TILE,
              (m + TILE - 1) / TILE);

    transposeTiled<<<grid, block>>>(d_in, d_out, m, n);

    cudaMemcpy(h_out, d_out, bytes, cudaMemcpyDeviceToHost);

    cout << h_in[2][7] << " should equal " << h_out[7][2] << "\n";

    cudaFree(d_in);
    cudaFree(d_out);
    return 0;
}

