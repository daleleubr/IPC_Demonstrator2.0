#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <chrono>
#include "common.h"

#pragma comment(lib, "ws2_32.lib")

void runServer() {
    // Implementação simplificada do servidor para testes
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) return;

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        closesocket(server_socket);
        return;
    }

    listen(server_socket, 1);

    sockaddr_in client_addr{};
    int client_size = sizeof(client_addr);
    SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_size);

    if (client_socket != INVALID_SOCKET) {
        char buffer[1024];
        int bytes = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes > 0) {
            send(client_socket, "response", 9, 0);
        }
        closesocket(client_socket);
    }

    closesocket(server_socket);
}

int main() {
    std::cout << "=== TESTE INTEGRATION ===" << std::endl;

    if (!init_winsock()) {
        std::cout << "ERRO: Falha na inicialização do Winsock!" << std::endl;
        return 1;
    }

    // Inicia servidor em thread separada
    std::thread server_thread(runServer);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Testa cliente
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cout << "ERRO: Falha ao criar socket do cliente!" << std::endl;
        server_thread.join();
        cleanup_winsock();
        return 1;
    }

    sockaddr_in hint{};
    hint.sin_family = AF_INET;
    hint.sin_port = htons(PORT);
    hint.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (sockaddr*)&hint, sizeof(hint)) != SOCKET_ERROR) {
        std::cout << "✓ Conectado ao servidor" << std::endl;

        // Envia mensagem
        if (send(sock, "test", 5, 0) > 0) {
            std::cout << "✓ Mensagem enviada" << std::endl;

            // Recebe resposta
            char buffer[1024];
            int bytes = recv(sock, buffer, sizeof(buffer), 0);
            if (bytes > 0) {
                std::cout << "✓ Resposta recebida: " << buffer << std::endl;
            }
            else {
                std::cout << "ERRO: Nenhuma resposta recebida" << std::endl;
            }
        }
    }
    else {
        std::cout << "ERRO: Falha na conexão com o servidor" << std::endl;
    }

    closesocket(sock);
    server_thread.join();
    cleanup_winsock();

    std::cout << "=== TESTE INTEGRATION COMPLETO ===" << std::endl;
    return 0;
}