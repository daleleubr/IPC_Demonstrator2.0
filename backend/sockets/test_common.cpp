#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "common.h"

#pragma comment(lib, "ws2_32.lib")

int main() {
    std::cout << "=== TESTE COMMON.H ===" << std::endl;

    // Teste make_json
    std::cout << "\n1. Teste make_json:" << std::endl;
    std::string json = make_json("status", "test message");
    std::cout << "Resultado: " << json << std::endl;

    // Teste make_json com caracteres especiais
    std::string json2 = make_json("error", "conexão falhou");
    std::cout << "Com caracteres especiais: " << json2 << std::endl;

    // Teste Winsock
    std::cout << "\n2. Teste Winsock:" << std::endl;
    if (init_winsock()) {
        std::cout << "Winsock inicializado com sucesso!" << std::endl;

        // Teste constante PORT
        std::cout << "PORT = " << PORT << std::endl;

        cleanup_winsock();
        std::cout << "Winsock finalizado!" << std::endl;
    }
    else {
        std::cout << "Falha na inicialização do Winsock!" << std::endl;
        return 1;
    }

    std::cout << "\n=== TODOS OS TESTES PASSARAM! ===" << std::endl;
    return 0;
}