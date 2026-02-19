// Conv1D forward, stride=1, padding=P, dilation=1
// X: (N,Cin,L), W: (Cout,Cin,K), Y: (N,Cout,Lout)
// row-major contiguous: ((n*Cin + ci)*L + x)

__global__ void conv1d_forward(const float* __restrict__ X,
                               const float* __restrict__ W,
                               const float* __restrict__ B, // can be nullptr
                               float* __restrict__ Y,
                               int N, int Cin, int L,
                               int Cout, int K, int P) {
    int n  = blockIdx.z;
    int co = blockIdx.y;
    int p  = blockIdx.x * blockDim.x + threadIdx.x; // output position

    int Lout = L + 2*P - K + 1;
    if (p >= Lout) return;

    float acc = (B ? B[co] : 0.0f);

    // sum over channels and kernel taps
    for (int ci = 0; ci < Cin; ++ci) {
        for (int k = 0; k < K; ++k) {
            int x_idx = p + k - P;   // input position
            float x = (0 <= x_idx && x_idx < L) ? X[(n*Cin + ci)*L + x_idx] : 0.0f;
            float w = W[(co*Cin + ci)*K + k];
            acc += x * w;
        }
    }

    Y[(n*Cout + co)*Lout + p] = acc;
}
