#include <iostream>
#include <vector>
#include <cuda_runtime.h>

#define CHECK_CUDA(call) \
    { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            std::cerr << "CUDA Error: " << cudaGetErrorString(err) << std::endl; \
            exit(1); \
        } \
    }

// ---------------- KERNEL ----------------
__global__ void conv2d_forward_naive(
    const float* X,
    const float* W,
    float* Y,
    int N, int Cin, int H, int W_in,
    int Cout, int Kh, int Kw,
    int Ph, int Pw)
{
    int n  = blockIdx.z;
    int co = blockIdx.y;

    int Hout = H + 2*Ph - Kh + 1;
    int Wout = W_in + 2*Pw - Kw + 1;

    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= Hout * Wout) return;

    int i = idx / Wout;
    int j = idx % Wout;

    float acc = 0.0f;

    for (int ci = 0; ci < Cin; ++ci) {
        for (int u = 0; u < Kh; ++u) {
            for (int v = 0; v < Kw; ++v) {
                int x_i = i + u - Ph;
                int x_j = j + v - Pw;

                float x = 0.0f;
                if (0 <= x_i && x_i < H && 0 <= x_j && x_j < W_in) {
                    x = X[((n*Cin + ci)*H + x_i)*W_in + x_j];
                }

                float w = W[(((co*Cin + ci)*Kh + u)*Kw + v)];
                acc += x * w;
            }
        }
    }

    Y[((n*Cout + co)*Hout + i)*Wout + j] = acc;
}

// ---------------- MAIN ----------------
int main() {

    // Small example
    int N = 1;
    int Cin = 1;
    int H = 5;
    int W_in = 5;

    int Cout = 1;
    int Kh = 3;
    int Kw = 3;

    int Ph = 1;  // padding
    int Pw = 1;

    int Hout = H + 2*Ph - Kh + 1;
    int Wout = W_in + 2*Pw - Kw + 1;

    size_t sizeX = N * Cin * H * W_in * sizeof(float);
    size_t sizeW = Cout * Cin * Kh * Kw * sizeof(float);
    size_t sizeY = N * Cout * Hout * Wout * sizeof(float);

    // Host memory
    std::vector<float> h_X(N * Cin * H * W_in, 1.0f);
    std::vector<float> h_W(Cout * Cin * Kh * Kw, 1.0f);
    std::vector<float> h_Y(N * Cout * Hout * Wout);

    // Device memory
    float *d_X, *d_W, *d_Y;

    CHECK_CUDA(cudaMalloc(&d_X, sizeX));
    CHECK_CUDA(cudaMalloc(&d_W, sizeW));
    CHECK_CUDA(cudaMalloc(&d_Y, sizeY));

    CHECK_CUDA(cudaMemcpy(d_X, h_X.data(), sizeX, cudaMemcpyHostToDevice));
    CHECK_CUDA(cudaMemcpy(d_W, h_W.data(), sizeW, cudaMemcpyHostToDevice));

    // Launch kernel
    int total = Hout * Wout;
    dim3 block(256);
    dim3 grid((total + block.x - 1) / block.x, Cout, N);

    conv2d_forward_naive<<<grid, block>>>(
        d_X, d_W, d_Y,
        N, Cin, H, W_in,
        Cout, Kh, Kw,
        Ph, Pw
    );

    CHECK_CUDA(cudaDeviceSynchronize());

    // Copy result back
    CHECK_CUDA(cudaMemcpy(h_Y.data(), d_Y, sizeY, cudaMemcpyDeviceToHost));

    // Print output
    std::cout << "Output:\n";
    for (int i = 0; i < Hout; ++i) {
        for (int j = 0; j < Wout; ++j) {
            std::cout << h_Y[i * Wout + j] << " ";
        }
        std::cout << "\n";
    }

    cudaFree(d_X);
    cudaFree(d_W);
    cudaFree(d_Y);

    return 0;
}
