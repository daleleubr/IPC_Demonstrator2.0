// Pipes_Final_Pai.cpp : Este arquivo contém a função 'main'. A execução do programa começa e termina ali.

#include <iostream>    // Para entrada/saída padrão (cout, cin, endl)
#include <string>      // Para usar a classe string
#include <windows.h>   // API do Windows (para CreatePipe, CreateProcess, etc.)
#include <stdio.h>     // Para funções de entrada/saída padrão em C
#include <tchar.h>     // Para lidar com caracteres Unicode/ANSI

#define BUFSIZE 512  // Tamanho do buffer para leitura/escrita

int main()
{
    HANDLE hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES saAttr;  // Atributos de segurança do pipe
    BOOL fSuccess;               // Para verificar sucesso das operações
    CHAR chBuf[BUFSIZE];         // Buffer para armazenar dados
    DWORD dwRead, dwWritten;     // Número de bytes lidos/escritos

    std::cout << "=== DEMONSTRACAO DE PIPES ANONIMOS NO WINDOWS ===" << std::endl;
    std::cout << "==================================================" << std::endl << std::endl;

    // Configurar os atributos de segurança para o pipe
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;  // Handles são herdáveis
    saAttr.lpSecurityDescriptor = NULL;

    // Passo 1: Criar o pipe anônimo
    std::cout << "1. Criando pipe anonimo..." << std::endl;

    fSuccess = CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0);

    // Verificar se a criação do pipe foi bem-sucedida
    if (!fSuccess) {
        std::cerr << "ERRO: CreatePipe falhou. Código: " << GetLastError() << std::endl;
        return 1;
    }

    // Exibir informações sobre o pipe criado
    std::cout << "    Pipe criado com sucesso!" << std::endl;
    std::cout << "   - Handle de leitura: " << hReadPipe << std::endl;
    std::cout << "   - Handle de escrita: " << hWritePipe << std::endl << std::endl;

    // Passo 2: Criar o processo filho
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;
    TCHAR szCmdline[] = TEXT("notepad.exe C:\\Windows\\System32\\drivers\\etc\\hosts"); //TCHAR szCmdline[] = TEXT("child_process");    

    // Inicializar estruturas
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));

    // Configurar o STARTUPINFO para redirecionar os handles padrão
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    siStartInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    siStartInfo.hStdInput = hReadPipe;  // O filho lerá do pipe
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    // Criar o processo filho
    fSuccess = CreateProcess(
        NULL,          // Nome do aplicativo (NULL = usar linha de comando)
        szCmdline,     // Linha de comando
        NULL,          // Atributos de segurança do processo
        NULL,          // Atributos de segurança da thread
        TRUE,          // Handles são herdáveis
        0,             // Flags de criação
        NULL,          // Ambiente do pai
        NULL,          // Diretório do pai
        &siStartInfo,  // Informações de inicialização
        &piProcInfo    // Informações do processo
    );

    // Verificar se a criação do processo foi bem-sucedida
    if (!fSuccess) {
        std::cerr << "ERRO: CreateProcess falhou. Codigo: " << GetLastError() << std::endl;
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        return 1;
    }

    std::cout << "   Processo filho criado com sucesso! PID: " << piProcInfo.dwProcessId << std::endl << std::endl;

    // Fechar o handle de leitura no processo pai (o pai só ira escrever)
    CloseHandle(hReadPipe);

    // Passo 3: Escrever dados no pipe
    std::string mensagem = "Ola processo filho! Esta mensagem veio atraves do pipe!";

    // Escrever a mensagem no pipe
    fSuccess = WriteFile(
        hWritePipe,           // Handle do pipe para escrita
        mensagem.c_str(),     // Dados a serem escritos
        mensagem.size() + 1,  // Tamanho dos dados (incluindo null terminator)
        &dwWritten,           // Número de bytes escritos
        NULL                  // Overlapped I/O (não usado)
    );

    // Verificar se a escrita foi bem-sucedida
    if (!fSuccess) {
        std::cerr << "ERRO: WriteFile falhou. Código: " << GetLastError() << std::endl;
        CloseHandle(hWritePipe);
        return 1;
    }

    std::cout << "    " << dwWritten << " bytes escritos no pipe" << std::endl;
    std::cout << "   - Mensagem: \"" << mensagem << "\"" << std::endl << std::endl;

    // Passo 4: Fechar o handle de escrita para sinalizar EOF ao processo filho
    std::cout << "4. Fechando handle de escrita..." << std::endl;
    CloseHandle(hWritePipe);
    std::cout << "    Handle de escrita fechado" << std::endl;
    std::cout << "   - Isso sinaliza EOF para o processo filho" << std::endl << std::endl;

    // Passo 5: Esperar o processo filho terminar
    std::cout << "5. Esperando processo filho terminar..." << std::endl;
    WaitForSingleObject(piProcInfo.hProcess, INFINITE);

    // Limpeza de recursos 
    std::cout << "6. Limpando recursos..." << std::endl;
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);
    std::cout << "   Handles do processo fechados" << std::endl << std::endl;


    std::cout << "==================================================" << std::endl;
    std::cout << "=== DEMONSTRACAO CONCLUIDA COM SUCESSO! ===" << std::endl;
    return 0;
}


