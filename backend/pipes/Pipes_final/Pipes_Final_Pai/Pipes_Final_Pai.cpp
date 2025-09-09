#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>

// ===================== util: json =====================
static std::string json_escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 16);
    for (unsigned char c : s) {
        switch (c) {
        case '\"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\b': out += "\\b";  break;
        case '\f': out += "\\f";  break;
        case '\n': out += "\\n";  break;
        case '\r': out += "\\r";  break;
        case '\t': out += "\\t";  break;
        default:
            if (c < 0x20) {
                char buf[7];
                std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                out += buf;
            }
            else {
                out += static_cast<char>(c);
            }
        }
    }
    return out;
}

static void log_json(const char* type, const std::string& message) {
    std::cout << "{\"source\":\"PIPES-PAI\",\"type\":\"" << type
        << "\",\"message\":\"" << json_escape(message) << "\"}"
        << std::endl;
}
// ======================================================

static std::string join_args(int argc, char** argv, int start) {
    std::string out;
    for (int i = start; i < argc; ++i) {
        if (i > start) out += ' ';
        out += argv[i];
    }
    return out;
}

int main(int argc, char** argv) {
    const char* PIPE_NAME = R"(\\.\pipe\ipc_demo_pipe)";

    // mensagem do front (ou padrão)
    std::string mensagem = (argc >= 2) ? join_args(argc, argv, 1)
        : "OlaMundoDoPipe";

    // Cria o named pipe (duplex, 1 instância)
    HANDLE hPipe = CreateNamedPipeA(
        PIPE_NAME,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        1,                  // n. de instâncias
        4096, 4096,         // out/in buffer
        5000,               // default timeout (ms)
        nullptr
    );
    if (hPipe == INVALID_HANDLE_VALUE) {
        log_json("error", "CreateNamedPipe falhou (err=" + std::to_string(GetLastError()) + ")");
        return 1;
    }

    log_json("status", std::string("Aguardando conexão no pipe: ") + PIPE_NAME);

    BOOL ok = ConnectNamedPipe(hPipe, nullptr);
    if (!ok) {
        DWORD e = GetLastError();
        if (e != ERROR_PIPE_CONNECTED) {
            log_json("error", "ConnectNamedPipe falhou (err=" + std::to_string(e) + ")");
            CloseHandle(hPipe);
            return 1;
        }
        // se ERROR_PIPE_CONNECTED, já está conectado
    }
    log_json("status", "Cliente conectado");

    // --- Envia a mensagem do front ---
    DWORD written = 0;
    ok = WriteFile(hPipe, mensagem.c_str(), (DWORD)mensagem.size(), &written, nullptr);
    if (!ok) {
        log_json("error", "WriteFile (PAI) falhou (err=" + std::to_string(GetLastError()) + ")");
        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
        return 1;
    }
    log_json("sent", mensagem);

    // --- Lê resposta do FILHO (se houver) ---
    char buf[4096];
    DWORD readBytes = 0;
    ok = ReadFile(hPipe, buf, sizeof(buf), &readBytes, nullptr);
    if (ok && readBytes > 0) {
        std::string reply(buf, buf + readBytes);
        log_json("received", reply);
    }
    else {
        DWORD e = GetLastError();
        // Se o cliente fechou sem enviar nada, não é necessariamente erro fatal
        if (e != ERROR_BROKEN_PIPE && e != ERROR_MORE_DATA && readBytes > 0) {
            log_json("error", "ReadFile (PAI) falhou (err=" + std::to_string(e) + ")");
        }
    }

    FlushFileBuffers(hPipe);
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);

    log_json("status", "PAI finalizado");
    return 0;
}
