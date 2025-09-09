// Pipes_Final_Pai.cpp : Este arquivo contém a função 'main'. A execução do programa começa e termina ali.

#include <iostream>    // Para entrada/saída padrão (cout, cin, endl)
#include <string>      // Para usar a classe string
#include <windows.h>   // API do Windows (para CreatePipe, CreateProcess, etc.)
#include <stdio.h>     // Para funções de entrada/saída padrão em C
#include <tchar.h>     // Para lidar com caracteres Unicode/ANSI
#include "JsonFormatter.h"
#include <map>

#define BUFSIZE 512  // Tamanho do buffer para leitura/escrita

int main()
{
    HANDLE hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES saAttr;  // Atributos de segurança do pipe
    BOOL fSuccess;               // Para verificar sucesso das operações
    CHAR chBuf[BUFSIZE];         // Buffer para armazenar dados
    DWORD dwRead, dwWritten;     // Número de bytes lidos/escritos

    std::cout << "=== DEMONSTRACAO DE PIPES ANONIMOS NO WINDOWS ===" << std::endl;
    std::cout << JsonFormatter::formatEvent("demonstration_started",
        { {"message", "Demonstracao de Pipes Anonimos no Windows"} }) << std::endl;
    std::cout << "==================================================" << std::endl << std::endl;
    std::cout << JsonFormatter::formatEvent("section_separator") << std::endl;

    // Configurar os atributos de segurança para o pipe
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;  // Handles são herdáveis
    saAttr.lpSecurityDescriptor = NULL;

    // Passo 1: Criar o pipe anônimo
    std::cout << "1. Criando pipe anonimo..." << std::endl;
    std::cout << JsonFormatter::formatEvent("pipe_creation_started") << std::endl;

    // Inicialize os handles antes de usá-los para evitar o uso de valores indefinidos
    hReadPipe = NULL;
    hWritePipe = NULL;

    fSuccess = CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0);

    // Verificar se a criação do pipe foi bem-sucedida
    if (!fSuccess) {
        std::cerr << JsonFormatter::formatError("pipe_creation_failed",
            { {"error_code", std::to_string(GetLastError())} }) << std::endl;
        return 1;
    }

    // Exibir informações sobre o pipe criado
    std::cout << "    Pipe criado com sucesso!" << std::endl;
    std::cout << "   - Handle de leitura: " << hReadPipe << std::endl;
    std::cout << "   - Handle de escrita: " << hWritePipe << std::endl << std::endl;

    std::cout << JsonFormatter::formatEvent("pipe_created", {
    {"read_handle", std::to_string((long long)hReadPipe)},
    {"write_handle", std::to_string((long long)hWritePipe)}
        }) << std::endl;

    // Passo 2: Criar o processo filho
    std::cout << "2. Criando processo filho..." << std::endl;
    std::cout << JsonFormatter::formatEvent("child_process_creation_started") << std::endl;
    // Estruturas para criar o processo filho
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;
    TCHAR szCmdline[] = TEXT("Pipes_Final_FIlho.exe");
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
        std::cerr << JsonFormatter::formatError("child_process_creation_failed - Error code: " +
            std::to_string(GetLastError())) << std::endl;
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        return 1;
    }

    std::cout << "   Processo filho criado com sucesso! PID: " << piProcInfo.dwProcessId << std::endl << std::endl;
    std::cout << JsonFormatter::formatEvent("child_process_created", {
    {"child_pid", std::to_string(piProcInfo.dwProcessId)}
        }) << std::endl;

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
        std::cerr << JsonFormatter::formatError("write_to_pipe_failed - Error code: " +
            std::to_string(GetLastError())) << std::endl;
        CloseHandle(hWritePipe);
        return 1;
    }

    std::cout << "    " << dwWritten << " bytes escritos no pipe" << std::endl;
    std::cout << "   - Mensagem: \"" << mensagem << "\"" << std::endl << std::endl;
    std::cout << JsonFormatter::formatEvent("data_written", {
    {"bytes_written", std::to_string(dwWritten)},
    {"message", mensagem}
        }) << std::endl;

    // Passo 4: Fechar o handle de escrita para sinalizar EOF ao processo filho

    std::cout << JsonFormatter::formatEvent("pipe_write_end_closed") << std::endl;
    std::cout << "4. Fechando handle de escrita..." << std::endl;
    CloseHandle(hWritePipe);
    std::cout << "    Handle de escrita fechado" << std::endl;
    std::cout << "   - Isso sinaliza EOF para o processo filho" << std::endl << std::endl;

    // Passo 5: Esperar o processo filho terminar
    std::cout << JsonFormatter::formatEvent("waiting_for_child_exit") << std::endl;
    std::cout << "5. Esperando processo filho terminar..." << std::endl;
    WaitForSingleObject(piProcInfo.hProcess, INFINITE);
    std::cout << JsonFormatter::formatEvent("child_process_exited") << std::endl;
    std::cout << "   Processo filho finalizado" << std::endl << std::endl;

    // Limpeza de recursos 
    std::cout << JsonFormatter::formatEvent("cleaning_resources") << std::endl;
    std::cout << "6. Limpando recursos..." << std::endl;
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);
    std::cout << "   Handles do processo fechados" << std::endl << std::endl;


    std::cout << "==================================================" << std::endl;
    std::cout << "=== DEMONSTRACAO CONCLUIDA COM SUCESSO! ===" << std::endl;
    return 0;
}
// ============================================================================
// IMPLEMENTAÇÃO DO JSONFORMATER 
// ============================================================================

std::string JsonFormatter::formatEvent(const std::string& eventType,
    const std::map<std::string, std::string>& details) {

    std::stringstream json;
    json << "{\"type\":\"event\",\"event\":\"" << escapeJsonString(eventType)
        << "\",\"timestamp\":\"" << getCurrentTimestamp() << "\"";

    if (!details.empty()) {
        json << ",\"details\":{";
        bool first = true;
        for (const auto& pair : details) {
            if (!first) json << ",";
            json << "\"" << escapeJsonString(pair.first) << "\":\""
                << escapeJsonString(pair.second) << "\"";
            first = false;
        }
        json << "}";
    }
    json << "}";
    return json.str();
}

std::string JsonFormatter::formatError(const std::string& errorMessage,
    const std::map<std::string, std::string>& details) {
    std::stringstream json;
    json << "{\"type\":\"error\",\"timestamp\":\"" << getCurrentTimestamp()
        << "\",\"message\":\"" << escapeJsonString(errorMessage) << "\"";

    if (!details.empty()) {
        json << ",\"details\":{";
        bool first = true;
        for (const auto& pair : details) {
            if (!first) json << ",";
            json << "\"" << escapeJsonString(pair.first) << "\":\""
                << escapeJsonString(pair.second) << "\"";
            first = false;
        }
        json << "}";
    }
    json << "}";
    return json.str();
}

std::string JsonFormatter::getCurrentTimestamp() {
    auto now = std::time(nullptr);
    std::tm localTime;
    localtime_s(&localTime, &now);

    std::stringstream ss;
    ss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string JsonFormatter::escapeJsonString(const std::string& input) {
    std::string output;
    output.reserve(input.length());

    for (char c : input) {
        switch (c) {
        case '"':  output += "\\\""; break;
        case '\\': output += "\\\\"; break;
        case '\b': output += "\\b"; break;
        case '\f': output += "\\f"; break;
        case '\n': output += "\\n"; break;
        case '\r': output += "\\r"; break;
        case '\t': output += "\\t"; break;
        default:   output += c; break;
        }
    }
    return output;
}

