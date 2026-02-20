#include <iostream>
#include <vector>
#include <cuda_runtime.h>

/*
    Macro to check CUDA API errors.
    If any CUDA call fails, print error and exit.
*/
#define CHECK_CUDA(call) \
    { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            std::cerr << "CUDA Error: " << cudaGetErrorString(err) << std::endl; \
            exit(1); \
        } \
    }

/*
==========================================================
 NAIVE 2D CONVOLUTION KERNEL
==========================================================

Tensor layouts used (row-major contiguous memory):

Input X:
    [N, Cin, H, W]

Weights W:
    [Cout, Cin, Kh, Kw]

Output Y:
    [N, Cout, Hout, Wout]

Each thread computes ONE output pixel:
    Y[n, co, i, j]

Grid layout:
    blockIdx.z → batch index (n)
    blockIdx.y → output channel (co)
    blockIdx.x → flattened spatial index (i, j)
==========================================================
*/
__global__ void conv2d_forward_naive(
    const float* X,   // Input tensor
    const float* W,   // Weight tensor
    float* Y,         // Output tensor
    int N, int Cin, int H, int W_in,
    int Cout, int Kh, int Kw,
    int Ph, int Pw)   // Padding height and width
{
    // -----------------------------
    // Identify batch and output channel
    // -----------------------------
    int n  = blockIdx.z;   // Batch index
    int co = blockIdx.y;   // Output channel index

    // Compute output spatial dimensions
    int Hout = H + 2*Ph - Kh + 1;
    int Wout = W_in + 2*Pw - Kw + 1;

    // Flattened spatial index handled by threads
    int idx = blockIdx.x * blockDim.x + threadIdx.x;

    // Guard against out-of-bounds threads
    if (idx >= Hout * Wout) return;

    // Convert flattened index to 2D output coordinate
    int i = idx / Wout;   // row
    int j = idx % Wout;   // column

    float acc = 0.0f;     // Accumulator for convolution result

    /*
        Convolution formula:

        Y[n, co, i, j] =
            sum over (ci, u, v)
            X[n, ci, i+u-Ph, j+v-Pw] * W[co, ci, u, v]
    */

    // Loop over input channels
    for (int ci = 0; ci < Cin; ++ci) {

        // Loop over kernel height
        for (int u = 0; u < Kh; ++u) {

            // Loop over kernel width
            for (int v = 0; v < Kw; ++v) {

                // Compute input spatial location
                int x_i = i + u - Ph;
                int x_j = j + v - Pw;

                float x = 0.0f;

                // Handle padding boundary condition
                if (0 <= x_i && x_i < H &&
                    0 <= x_j && x_j < W_in) {

                    // Compute flattened index for X:
                    // ((n * Cin + ci) * H + x_i) * W + x_j
                    x = X[((n*Cin + ci)*H + x_i)*W_in + x_j];
                }

                // Compute flattened index for W:
                // (((co * Cin + ci) * Kh + u) * Kw + v)
                float w = W[(((co*Cin + ci)*Kh + u)*Kw + v)];

                acc += x * w;
            }
        }
    }

    // Write output value
    Y[((n*Cout + co)*Hout + i)*Wout + j] = acc;
}


// ==========================================================
// MAIN FUNCTION
// ==========================================================
int main() {

    /*
        Example:
        1 batch
        1 input channel
        5x5 input
        3x3 kernel
        padding = 1
    */

    int N = 1;
    int Cin = 1;
    int H = 5;
    int W_in = 5;

    int Cout = 1;
    int Kh = 3;
    int Kw = 3;

    int Ph = 1;  // padding height
    int Pw = 1;  // padding width

    // Compute output dimensions
    int Hout = H + 2*Ph - Kh + 1;
    int Wout = W_in + 2*Pw - Kw + 1;

    // Compute memory sizes
    size_t sizeX = N * Cin * H * W_in * sizeof(float);
    size_t sizeW = Cout * Cin * Kh * Kw * sizeof(float);
    size_t sizeY = N * Cout * Hout * Wout * sizeof(float);

    // Allocate host memory
    std::vector<float> h_X(N * Cin * H * W_in, 1.0f);
    std::vector<float> h_W(Cout * Cin * Kh * Kw, 1.0f);
    std::vector<float> h_Y(N * Cout * Hout * Wout);

    // Allocate device memory
    float *d_X, *d_W, *d_Y;

    CHECK_CUDA(cudaMalloc(&d_X, sizeX));
    CHECK_CUDA(cudaMalloc(&d_W, sizeW));
    CHECK_CUDA(cudaMalloc(&d_Y, sizeY));

    // Copy input and weights to GPU
    CHECK_CUDA(cudaMemcpy(d_X, h_X.data(), sizeX, cudaMemcpyHostToDevice));
    CHECK_CUDA(cudaMemcpy(d_W, h_W.data(), sizeW, cudaMemcpyHostToDevice));

    /*
        Launch configuration:

        Each thread computes one output pixel.

        grid.x → spatial output
        grid.y → output channels
        grid.z → batch
    */

    int total = Hout * Wout;

    dim3 block(256);  // 256 threads per block

    dim3 grid(
        (total + block.x - 1) / block.x,  // cover spatial
        Cout,
        N
    );

    // Launch kernel
    conv2d_forward_naive<<<grid, block>>>(
        d_X, d_W, d_Y,
        N, Cin, H, W_in,
        Cout, Kh, Kw,
        Ph, Pw
    );

    CHECK_CUDA(cudaDeviceSynchronize());

    // Copy output back to host
    CHECK_CUDA(cudaMemcpy(h_Y.data(), d_Y, sizeY, cudaMemcpyDeviceToHost));

    // Print output
    std::cout << "Output:\n";
    for (int i = 0; i < Hout; ++i) {
        for (int j = 0; j < Wout; ++j) {
            std::cout << h_Y[i * Wout + j] << " ";
        }
        std::cout << "\n";
    }

    // Free device memory
    cudaFree(d_X);
    cudaFree(d_W);
    cudaFree(d_Y);

    return 0;
}
