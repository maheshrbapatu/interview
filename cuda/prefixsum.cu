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


/*
======================
UPSWEEP (reduce) phase
======================

Example: BLOCK = 4  =>  n = 2*BLOCK = 8 elements per block
Input chunk (shared memory s[]):

Index:  0 1 2 3 4 5 6 7
Value:  1 2 3 4 5 6 7 8

Goal of upsweep:
- Build a reduction tree so that s[n-1] holds TOTAL SUM of the block.
- This is like reduction, but stored in a tree layout.

stride = 1:
  idx = (tid+1)*2*1 - 1  => idx = 1,3,5,7 for tid=0..3
  s[1] += s[0]  => 2+1 = 3
  s[3] += s[2]  => 4+3 = 7
  s[5] += s[4]  => 6+5 = 11
  s[7] += s[6]  => 8+7 = 15
  s = [1,3,3,7,5,11,7,15]

stride = 2:
  idx = (tid+1)*2*2 - 1  => idx = 3,7 (tid=0..1)
  s[3] += s[1]  => 7+3  = 10
  s[7] += s[5]  => 15+11= 26
  s = [1,3,3,10,5,11,7,26]

stride = 4:
  idx = (tid+1)*2*4 - 1  => idx = 7 (tid=0 only)
  s[7] += s[3]  => 26+10 = 36
  s = [1,3,3,10,5,11,7,36]

After upsweep: s[7] = 36 (total sum of block chunk)


/*
Exclusive scan trick:
- After upsweep, s[n-1] contains total sum.
- To produce an EXCLUSIVE prefix sum, we set the root to 0.
  This means "sum of elements before the end" starts from 0.
*/

/*
=================
DOWNSWEEP phase
=================

We now convert the reduction tree into prefix sums.

State entering downsweep (after setting root to 0):
From upsweep we had: s = [1,3,3,10,5,11,7,36]
Set root to 0:        s = [1,3,3,10,5,11,7,0]

Downsweep works by "swap + accumulate" at each level.

stride = 4:
  idx = 7 (tid=0)
  t = s[3] (=10)
  s[3] = s[7] (=0)
  s[7] = s[7] + t => 0 + 10 = 10
  s = [1,3,3,0,5,11,7,10]

stride = 2:
  idx = 3 (tid=0), 7 (tid=1)
  idx=3:
    t=s[1]=3, s[1]=s[3]=0, s[3]+=t => 0+3=3
  idx=7:
    t=s[5]=11, s[5]=s[7]=10, s[7]+=t => 10+11=21
  s = [1,0,3,3,5,10,7,21]

stride = 1:
  idx = 1,3,5,7 (tid=0..3)
  idx=1:
    t=s[0]=1, s[0]=s[1]=0, s[1]+=t => 0+1=1
  idx=3:
    t=s[2]=3, s[2]=s[3]=3, s[3]+=t => 3+3=6
  idx=5:
    t=s[4]=5, s[4]=s[5]=10, s[5]+=t => 10+5=15
  idx=7:
    t=s[6]=7, s[6]=s[7]=21, s[7]+=t => 21+7=28

Final s (exclusive scan):
Index:  0 1 2 3  4  5  6  7
Value:  0 1 3 6 10 15 21 28

That matches exclusive prefix sums of [1 2 3 4 5 6 7 8].
*/
*/



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
