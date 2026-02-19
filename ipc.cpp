// ipc
int fd[2];
pipe(fd);  // fd[0] = read, fd[1] = write

if (fork() == 0) {
    // Child
    close(fd[1]);
    char buffer[100];
    read(fd[0], buffer, 100);
} else {
    // Parent
    close(fd[0]);
    write(fd[1], "hello", 5);
}


// shared memory
int fd = shm_open("/myshm", O_CREAT | O_RDWR, 0666);
ftruncate(fd, 4096);
void* ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

