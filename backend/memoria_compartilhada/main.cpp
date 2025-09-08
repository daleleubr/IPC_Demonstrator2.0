#include "shared_memory.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "{\"error\": \"Uso: shm_app.exe <comando> [dados]\"}" << std::endl;
        return 1;
    }

    SharedMemory shm("ipc_shm", 1024);
    if (!shm.initialize()) {
        std::cout << "{\"error\": \"Falha ao inicializar memória compartilhada\"}" << std::endl;
        return 1;
    }

    std::string command = argv[1];

    if (command == "write" && argc >= 3) {
        std::string data = argv[2];
        if (shm.write_data(data)) {
            std::cout << "{\"status\": \"success\", \"action\": \"write\", \"data\": \"" << data << "\"}" << std::endl;
        }
        else {
            std::cout << "{\"error\": \"Falha ao escrever dados\"}" << std::endl;
        }
    }
    else if (command == "read") {
        std::string data = shm.read_data();
        // Escape quotes for JSON
        size_t pos = 0;
        while ((pos = data.find('"', pos)) != std::string::npos) {
            data.replace(pos, 1, "\\\"");
            pos += 2;
        }
        std::cout << "{\"status\": \"success\", \"action\": \"read\", \"data\": \"" << data << "\"}" << std::endl;
    }
    else if (command == "status") {
        std::cout << shm.get_status_json() << std::endl;
    }
    else if (command == "clear") {
        shm.clear_memory();
        std::cout << "{\"status\": \"success\", \"action\": \"clear\"}" << std::endl;
    }
    else {
        std::cout << "{\"error\": \"Comando inválido: " << command << "\"}" << std::endl;
        return 1;
    }

    return 0;
}