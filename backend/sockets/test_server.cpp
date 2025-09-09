#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "common.h"

#pragma comment(lib, "ws2_32.lib")

int main() {
    std::cout << "=== TESTE SERVER REAL ===" << std::endl;

    if (!init_winsock()) {
        std::cout << "FALHA: Winsock não inicializou" << std::endl;
        return 1;
    }

    // Teste de criação de socket
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cout << "FALHA: Não criou socket" << std::endl;
        cleanup_winsock();
        return 1;
    }
    std::cout << "✓ Socket criado" << std::endl;

    // Teste de bind
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cout << "FALHA: Bind na porta " << PORT << " falhou" << std::endl;
        closesocket(server_socket);
        cleanup_winsock();
        return 1;
    }
    std::cout << "✓ Bind na porta " << PORT << " OK" << std::endl;

    closesocket(server_socket);
    cleanup_winsock();
    std::cout << "✓ Todos os testes SERVER passaram!" << std::endl;
    return 0;
}