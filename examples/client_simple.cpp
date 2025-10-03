#include <iostream>
#include <thread>
#include <string>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

void receive_messages(int server_fd) {
    char buffer[1024];
    while (true) {
        ssize_t bytes_received = recv(server_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            std::cout << "Desconectado do servidor.\n";
            close(server_fd);
            break;
        }
        buffer[bytes_received] = '\0';
        std::cout << "Servidor: " << buffer << std::endl;
    }
}

int main(int argc, char** argv) {
    std::string server_ip = "127.0.0.1";
    int port = 12345;
    if (argc >= 2) {
        server_ip = argv[1];
    }
    if (argc >= 3) {
        port = std::stoi(argv[2]);
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Falha ao criar socket.\n";
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);

    if (connect(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Falha ao conectar ao servidor.\n";
        return 1;
    }

    std::cout << "Conectado ao servidor.\n";

    std::thread(receive_messages, server_fd).detach();

    std::string message;
    while (true) {
        std::getline(std::cin, message);
        if (message == "/quit") {
            break;
        }
        send(server_fd, message.c_str(), message.size(), 0);
    }

    close(server_fd);
    return 0;
}