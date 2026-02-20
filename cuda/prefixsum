#include <iostream>
#include <vector>
#include <cuda_runtime.h>

#define CHECK(call)                                                    \
{                                                                      \
    cudaError_t err = call;                                            \
    if (err != cudaSuccess) {                                          \
        std::cerr << "CUDA error: " << cudaGetErrorString(err)         \
                  << " at line " << __LINE__ << std::endl;            \
        std::exit(1);                                                  \
    }                                                                  \
}

__global__ void scan_blelloch_exclusive(const float* __restrict__ in,
                                       float* __restrict__ out)
{
    extern __shared__ float s[];            // shared memory: 2*blockDim.x floats
    int tid = threadIdx.x;                  // 0 .. blockDim.x-1
    int n = 2 * blockDim.x;                 // elements per block
    int base = blockIdx.x * n;              // start index for this block's chunk

    // Load 2 elements per thread into shared memory
    s[tid] = in[base + tid];
    s[tid + blockDim.x] = in[base + tid + blockDim.x];
    __syncthreads();

    // UPSWEEP (reduce)
    for (int stride = 1; stride < n; stride <<= 1) {
        int idx = (tid + 1) * stride * 2 - 1;
        if (idx < n) {
            s[idx] += s[idx - stride];
        }
        __syncthreads();
    }

    // Set root to 0 for EXCLUSIVE scan
    if (tid == 0) s[n - 1] = 0.0f;
    __syncthreads();

    // DOWNSWEEP
    for (int stride = n >> 1; stride >= 1; stride >>= 1) {
        int idx = (tid + 1) * stride * 2 - 1;
        if (idx < n) {
            float t = s[idx - stride];
            s[idx - stride] = s[idx];
            s[idx] += t;
        }
        __syncthreads();
    }

    // Write results back to global memory
    out[base + tid] = s[tid];
    out[base + tid + blockDim.x] = s[tid + blockDim.x];
}

int main() {
    // For a simple demo, keep BLOCK small so output is readable
    const int BLOCK = 4;                 // blockDim.x
    const int elemsPerBlock = 2 * BLOCK; // elements scanned per block
    const int grid = 2;                  // number of blocks
    const int N = grid * elemsPerBlock;  // total elements (must be multiple)

    // Host input: two blocks worth of data
    // Block 0 chunk: 1 2 3 4 5 6 7 8
    // Block 1 chunk: 10 20 30 40 50 60 70 80
    std::vector<float> h_in = {
        1,2,3,4,5,6,7,8,
        10,20,30,40,50,60,70,80
    };
    std::vector<float> h_out(N, 0);

    float *d_in = nullptr, *d_out = nullptr;
    CHECK(cudaMalloc(&d_in,  N * sizeof(float)));
    CHECK(cudaMalloc(&d_out, N * sizeof(float)));

    CHECK(cudaMemcpy(d_in, h_in.data(), N * sizeof(float), cudaMemcpyHostToDevice));

    // Shared memory needed: elemsPerBlock floats
    size_t shmemBytes = elemsPerBlock * sizeof(float);

    scan_blelloch_exclusive<<<grid, BLOCK, shmemBytes>>>(d_in, d_out);
    CHECK(cudaGetLastError());
    CHECK(cudaDeviceSynchronize());

    CHECK(cudaMemcpy(h_out.data(), d_out, N * sizeof(float), cudaMemcpyDeviceToHost));

    // Print results block-by-block
    for (int b = 0; b < grid; b++) {
        std::cout << "Block " << b << " input : ";
        for (int i = 0; i < elemsPerBlock; i++)
            std::cout << h_in[b * elemsPerBlock + i] << " ";
        std::cout << "\n";

        std::cout << "Block " << b << " scan  : ";
        for (int i = 0; i < elemsPerBlock; i++)
            std::cout << h_out[b * elemsPerBlock + i] << " ";
        std::cout << "\n\n";
    }

    // Cleanup
    CHECK(cudaFree(d_in));
    CHECK(cudaFree(d_out));
    return 0;
}
