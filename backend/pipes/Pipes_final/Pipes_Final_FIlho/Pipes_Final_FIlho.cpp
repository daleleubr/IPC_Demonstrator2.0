// Pipes_Final_FIlho.cpp
#include <iostream>
#include <windows.h>
#include <stdio.h>  
#include <map>              
#include <string>
#include <sstream>

#define BUFSIZE 512

// Implementação JSON simplificada - DEVE VIR ANTES do main()
class JsonFormatter {
public:
    static std::string formatEvent(const std::string& eventType) {
        return "{\"type\":\"event\",\"event\":\"" + escapeJsonString(eventType) + "\"}";
    }

    static std::string formatEvent(const std::string& eventType, const std::map<std::string, std::string>& details) {
        std::stringstream json;
        json << "{\"type\":\"event\",\"event\":\"" << escapeJsonString(eventType) << "\"";

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

    static std::string formatError(const std::string& errorMessage) {
        return "{\"type\":\"error\",\"message\":\"" + escapeJsonString(errorMessage) + "\"}";
    }

    static std::string formatError(const std::string& errorMessage, const std::map<std::string, std::string>& details) {
        std::stringstream json;
        json << "{\"type\":\"error\",\"message\":\"" << escapeJsonString(errorMessage) << "\"";

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

private:
    static std::string escapeJsonString(const std::string& input) {
        std::string output;
        for (char c : input) {
            switch (c) {
            case '"':  output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            case '\n': output += "\\n"; break;
            case '\r': output += "\\r"; break;
            case '\t': output += "\\t"; break;
            default:   output += c; break;
            }
        }
        return output;
    }
};

int main() {
    std::cout << "[FILHO] Processo filho iniciado!" << std::endl;
    std::cout << JsonFormatter::formatEvent("child_process_started") << std::endl;

    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    CHAR chBuf[BUFSIZE];
    DWORD dwRead;
    BOOL fSuccess;

    // Ler dados do pipe (stdin)
    std::cout << "[FILHO] Lendo dados do pipe..." << std::endl;
    std::cout << JsonFormatter::formatEvent("reading_from_pipe") << std::endl;

    fSuccess = ReadFile(
        hStdIn,    // Handle de entrada (pipe)
        chBuf,     // Buffer para armazenar dados
        BUFSIZE,   // Tamanho do buffer
        &dwRead,   // Número de bytes lidos
        NULL       // Overlapped I/O (não usado)
    );

    if (!fSuccess || dwRead == 0) {
        std::string errorMsg = "[FILHO] ERRO: ReadFile falhou. Código: " + std::to_string(GetLastError());
        std::cerr << errorMsg << std::endl;

        std::map<std::string, std::string> errorDetails;
        errorDetails["error_code"] = std::to_string(GetLastError());
        std::cerr << JsonFormatter::formatError("read_from_pipe_failed", errorDetails) << std::endl;
        return 1;
    }

    // Mostrar dados recebidos
    std::cout << "[FILHO] " << dwRead << " bytes recebidos" << std::endl;
    std::cout << "[FILHO] Mensagem: \"" << chBuf << "\"" << std::endl;
    std::cout << "[FILHO] Processo filho finalizando..." << std::endl;

    std::map<std::string, std::string> eventDetails;
    eventDetails["bytes_received"] = std::to_string(dwRead);
    eventDetails["message"] = std::string(chBuf, dwRead);
    std::cout << JsonFormatter::formatEvent("data_received", eventDetails) << std::endl;

    return 0;
}