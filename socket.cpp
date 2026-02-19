#include <iostream>
#include <cstring>
#include <unistd.h>        // close()
#include <arpa/inet.h>     // sockaddr_in, inet_pton
#include <sys/socket.h>    // socket(), bind(), listen(), accept()

int main() {
    int port = 8080;

    // 1️⃣ Create socket
    // AF_INET  -> IPv4
    // SOCK_STREAM -> TCP
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        return 1;
    }

    // 2️⃣ Configure address structure
    sockaddr_in address;
    std::memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;         // IPv4
    address.sin_addr.s_addr = INADDR_ANY; // Accept connections on any IP
    address.sin_port = htons(port);       // Convert port to network byte order

    // 3️⃣ Bind socket to port
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return 1;
    }

    // 4️⃣ Start listening
    if (listen(server_fd, 5) < 0) {
        perror("listen failed");
        return 1;
    }

    std::cout << "Server listening on port " << port << "\n";

    // 5️⃣ Accept a client connection
    socklen_t addrlen = sizeof(address);
    int client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);
    if (client_fd < 0) {
        perror("accept failed");
        return 1;
    }

    std::cout << "Client connected!\n";

    // 6️⃣ Receive data
    char buffer[1024] = {0};
    int bytes = read(client_fd, buffer, sizeof(buffer));
    std::cout << "Received: " << buffer << "\n";

    // 7️⃣ Send response
    const char* msg = "Hello from server!";
    send(client_fd, msg, strlen(msg), 0);

    // 8️⃣ Close sockets
    close(client_fd);
    close(server_fd);

    return 0;
}
