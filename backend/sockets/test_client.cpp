#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "common.h"

#pragma comment(lib, "ws2_32.lib")

int main() {
    std::cout << "=== TESTE CLIENT ===" << std::endl;

    if (!init_winsock()) {
        std::cout << "ERRO: Falha na inicializacao do Winsock!" << std::endl;
        return 1;
    }

    // Teste 1: Criacao do socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cout << "ERRO: Falha ao criar socket!" << std::endl;
        cleanup_winsock();
        return 1;
    }
    std::cout << "✓ Socket criado com sucesso" << std::endl;

    // Teste 2: Tentativa de conexao com porta invalida
    sockaddr_in hint{};
    hint.sin_family = AF_INET;
    hint.sin_port = htons(9999); // Porta invalida
    hint.sin_addr.s_addr = inet_addr("127.0.0.1");

    int connect_result = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connect_result != SOCKET_ERROR) {
        std::cout << "ERRO: Conexao deveria ter falhado com porta invalida!" << std::endl;
        closesocket(sock);
        cleanup_winsock();
        return 1;
    }
    std::cout << "✓ Conexao com porta invalida falhou corretamente" << std::endl;

    closesocket(sock);
    cleanup_winsock();

    std::cout << "=== TODOS OS TESTES CLIENT PASSARAM! ===" << std::endl;
    return 0;
}