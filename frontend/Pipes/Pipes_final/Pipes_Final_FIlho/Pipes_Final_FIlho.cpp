// Pipes_Final_FIlho.cpp : Este arquivo contém a função 'main'. A execução do programa começa e termina ali.
//

#include <iostream>
#include <windows.h>
#include <stdio.h>

#define BUFSIZE 512

int main() {
    std::cout << "[FILHO] Processo filho iniciado!" << std::endl;

    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    CHAR chBuf[BUFSIZE];
    DWORD dwRead;
    BOOL fSuccess;

    // Ler dados do pipe (stdin)
    std::cout << "[FILHO] Lendo dados do pipe..." << std::endl;

    fSuccess = ReadFile(
        hStdIn,    // Handle de entrada (pipe)
        chBuf,     // Buffer para armazenar dados
        BUFSIZE,   // Tamanho do buffer
        &dwRead,   // Número de bytes lidos
        NULL       // Overlapped I/O (não usado)
    );

    if (!fSuccess || dwRead == 0) {
        std::cerr << "[FILHO] ERRO: ReadFile falhou. Código: " << GetLastError() << std::endl;
        return 1;
    }

    // Mostrar dados recebidos
    std::cout << "[FILHO] " << dwRead << " bytes recebidos" << std::endl;
    std::cout << "[FILHO] Mensagem: \"" << chBuf << "\"" << std::endl;
    std::cout << "[FILHO] Processo filho finalizando..." << std::endl;

    return 0;
}