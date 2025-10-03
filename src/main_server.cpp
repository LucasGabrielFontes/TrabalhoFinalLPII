#include "tslogger.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_set>
#include <string>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>

std::mutex clients_mutex;
std::unordered_set<int> clients;

void broadcast_message(const std::string& message, int sender_fd) {
    std::scoped_lock lock(clients_mutex);
    for (int client_fd : clients) {
        if (client_fd != sender_fd) {
            send(client_fd, message.c_str(), message.size(), 0);
        }
    }
}

void handle_client(int client_fd) {
    char buffer[1024];
    while (true) {
        ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            TSLog::info("Cliente desconectado: fd=%d", client_fd);
            close(client_fd);
            std::scoped_lock lock(clients_mutex);
            clients.erase(client_fd);
            break;
        }
        buffer[bytes_received] = '\0';
        TSLog::info("Mensagem recebida de fd=%d: %s", client_fd, buffer);
        broadcast_message(buffer, client_fd);
    }
}

int main(int argc, char** argv) {
    int port = 12345;
    if (argc >= 2) {
        port = std::stoi(argv[1]);
    }

    TSLog::Config cfg;
    cfg.path = "logs/server.log";
    cfg.min_level = TSLog::Level::Info;
    cfg.also_stderr = true;
    TSLog::init(cfg);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        TSLog::fatal("Falha ao criar socket: %s", strerror(errno));
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        TSLog::fatal("Falha ao fazer bind: %s", strerror(errno));
        return 1;
    }

    if (listen(server_fd, 10) == -1) {
        TSLog::fatal("Falha ao escutar: %s", strerror(errno));
        return 1;
    }

    TSLog::info("Servidor iniciado na porta %d", port);

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            TSLog::warn("Falha ao aceitar conexão: %s", strerror(errno));
            continue;
        }

        {
            std::scoped_lock lock(clients_mutex);
            clients.insert(client_fd);
        }

        TSLog::info("Novo cliente conectado: fd=%d", client_fd);
        std::thread(handle_client, client_fd).detach();
    }

    TSLog::close();
    return 0;
}