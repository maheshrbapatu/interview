#include <cuda_runtime.h>
#include <iostream>
using namespace std;

__global__ void vecAddition(int *a, int *b, int *c, int n) {
    int x = threadIdx.x + blockDim.x * blockIdx.x;
    if (x < n) {
        c[x] = a[x] + b[x];
    }
}

int main() {
    int n = 1000;
    int threadsPerBlock = 256;
    int blocks = (n + threadsPerBlock - 1) / threadsPerBlock;

    // Host arrays
    int *h_a = new int[n];
    int *h_b = new int[n];
    int *h_c = new int[n];

    for (int i = 0; i < n; i++) {
        h_a[i] = i;
        h_b[i] = i;
    }

    // Device pointers
    int *da, *db, *dc;
    size_t bytes = n * sizeof(int);

    cudaMalloc(&da, bytes);
    cudaMalloc(&db, bytes);
    cudaMalloc(&dc, bytes);

    cudaMemcpy(da, h_a, bytes, cudaMemcpyHostToDevice);
    cudaMemcpy(db, h_b, bytes, cudaMemcpyHostToDevice);

    vecAddition<<<blocks, threadsPerBlock>>>(da, db, dc, n);

    cudaDeviceSynchronize(); // wait for kernel

    cudaMemcpy(h_c, dc, bytes, cudaMemcpyDeviceToHost);

    // quick print
    cout << h_c[0] << " " << h_c[1] << " " << h_c[n-1] << endl;

    cudaFree(da);
    cudaFree(db);
    cudaFree(dc);

    delete[] h_a;
    delete[] h_b;
    delete[] h_c;

    return 0;
}

