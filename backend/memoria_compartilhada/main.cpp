#include "shared_memory.hpp"
#include <iostream>
#include <string>

static std::string escape_json(const std::string& s) {
    std::string result;
    for (char c : s) {
        switch (c) {
        case '"': result += "\\\""; break;
        case '\\': result += "\\\\"; break;
        case '\b': result += "\\b"; break;
        case '\f': result += "\\f"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default: result += c; break;
        }
    }
    return result;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "{\"error\":\"Uso: shm_app.exe <status|read|write|clear> [dados]\"}\n";
        return 1;
    }

    SharedMemory shm("ipc_shm", 1024);
    if (!shm.initialize()) {
        std::cout << "{\"error\":\"Falha ao inicializar memoria compartilhada\"}\n";
        return 1;
    }

    std::string command = argv[1];

    if (command == "write" && argc >= 3) {
        std::string data = argv[2];
        if (shm.write_data(data)) {
            std::cout << "{\"ok\":true,\"event\":\"write\",\"data\":\"" << escape_json(data) << "\"}\n";
        }
        else {
            std::cout << "{\"ok\":false,\"event\":\"write\",\"error\":\"Falha ao escrever dados\"}\n";
        }
    }
    else if (command == "read") {
        std::string data = shm.read_data();
        std::cout << "{\"ok\":true,\"event\":\"read\",\"data\":\"" << escape_json(data) << "\"}\n";
    }
    else if (command == "status") {
        std::cout << shm.get_status_json() << "\n";
    }
    else if (command == "clear") {
        shm.clear_memory();
        std::cout << "{\"ok\":true,\"event\":\"clear\"}\n";
    }
    else {
        std::cout << "{\"ok\":false,\"error\":\"Comando invalido: " << escape_json(command) << "\"}\n";
        return 1;
    }

    return 0;
}
