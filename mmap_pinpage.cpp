#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>

int main() {
    size_t length = 10 * 1024 * 1024; // 10MB

    void* addr = mmap(nullptr,
                      length,
                      PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS,
                      -1,
                      0);

    if (addr == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    // Touch pages so they are allocated
    memset(addr, 0, length);

    // Pin memory
    if (mlock(addr, length) != 0) {
        perror("mlock failed");
        return 1;
    }

    std::cout << "Memory pinned successfully\n";

    getchar(); // keep alive

    munlock(addr, length);
    munmap(addr, length);

    return 0;
}
