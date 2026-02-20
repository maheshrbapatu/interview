#include <iostream>
#include <vector>
#include <cuda_runtime.h>

#define CHECK(call)                                                   \
{                                                                     \
    const cudaError_t error = call;                                   \
    if (error != cudaSuccess) {                                       \
        std::cerr << "Error: " << cudaGetErrorString(error)           \
                  << " at line " << __LINE__ << std::endl;            \
        exit(1);                                                       \
    }                                                                 \
}

/*
    Kernel: Block-level parallel reduction.
    Each block reduces its portion of the input array
    and writes one value to the output array.
*/
__global__ void reduce(float* input, float* output, int n)
{
    // Dynamically allocated shared memory
    extern __shared__ float sdata[];

    // Thread ID within block
    unsigned int tid = threadIdx.x;

    // Global index
    unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;

    // Load input into shared memory
    // If out of bounds, load 0
    if (i < n)
        sdata[tid] = input[i];
    else
        sdata[tid] = 0.0f;

    __syncthreads();  // Ensure all threads loaded data

    /*
        Reduction in shared memory
        Each iteration halves active threads
    */
    for (unsigned int s = blockDim.x / 2; s > 0; s >>= 1)
    {
        if (tid < s)
        {
            sdata[tid] += sdata[tid + s];
        }

        __syncthreads();  // Wait before next reduction step
    }

    // First thread writes block result to global memory
    if (tid == 0)
    {
        output[blockIdx.x] = sdata[0];
    }
}

int main()
{
    const int N = 1 << 20;  // 1M elements
    const int blockSize = 256;
    const int gridSize = (N + blockSize - 1) / blockSize;

    std::cout << "Array size: " << N << std::endl;
    std::cout << "Grid size: " << gridSize << std::endl;
    std::cout << "Block size: " << blockSize << std::endl;

    // Host memory
    std::vector<float> h_input(N, 1.0f);  // Fill with 1s
    std::vector<float> h_output(gridSize);

    float* d_input;
    float* d_output;

    // Allocate device memory
    CHECK(cudaMalloc(&d_input, N * sizeof(float)));
    CHECK(cudaMalloc(&d_output, gridSize * sizeof(float)));

    // Copy input to device
    CHECK(cudaMemcpy(d_input, h_input.data(),
                     N * sizeof(float),
                     cudaMemcpyHostToDevice));

    /*
        Launch kernel
        Shared memory size = blockSize * sizeof(float)
    */
    reduce<<<gridSize, blockSize, blockSize * sizeof(float)>>>
        (d_input, d_output, N);

    CHECK(cudaDeviceSynchronize());

    // Copy partial sums back
    CHECK(cudaMemcpy(h_output.data(), d_output,
                     gridSize * sizeof(float),
                     cudaMemcpyDeviceToHost));

    // Final reduction on CPU
    float finalSum = 0.0f;
    for (int i = 0; i < gridSize; i++)
        finalSum += h_output[i];

    std::cout << "GPU reduction result: " << finalSum << std::endl;
    std::cout << "Expected result: " << N << std::endl;

    // Cleanup
    CHECK(cudaFree(d_input));
    CHECK(cudaFree(d_output));

    return 0;
}
