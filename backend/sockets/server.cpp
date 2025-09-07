#include "common.h"

// Servidor Socket
// HENRIQUE - Sistema de comunicação via sockets

int main() {
    setlocale(LC_ALL, "Portuguese_Brazil.1252");  // ← CORREÇÃO DOS CARACTERES

    if (!init_winsock()) return 1;  

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << make_json("error", "Não foi possível criar socket servidor") << std::endl;
        cleanup_winsock();
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << make_json("error", "Falha no bind") << std::endl;
        closesocket(server_socket);
        cleanup_winsock();
        return 1;
    }

    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << make_json("error", "Falha no listen") << std::endl;
        closesocket(server_socket);
        cleanup_winsock();
        return 1;
    }

    std::cout << make_json("status", "Servidor aguardando conexões na porta " + std::to_string(PORT)) << std::endl;

    sockaddr_in client_addr{};
    int client_size = sizeof(client_addr);
    SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_size);

    if (client_socket == INVALID_SOCKET) {
        std::cerr << make_json("error", "Falha ao aceitar conexão") << std::endl;
        closesocket(server_socket);
        cleanup_winsock();
        return 1;
    }

    std::cout << make_json("status", "Cliente conectado") << std::endl;

    char buffer[4096];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

    if (bytes_received > 0) {
        std::string message(buffer, bytes_received);
        std::cout << make_json("received", message) << std::endl;

        std::string response = "Servidor recebeu: " + message;
        send(client_socket, response.c_str(), response.size(), 0);
        std::cout << make_json("sent", response) << std::endl;
    }

    closesocket(client_socket);
    closesocket(server_socket);
    cleanup_winsock();

    std::cout << make_json("status", "Servidor finalizado") << std::endl;
    return 0;
}