#include "shared_memory.hpp"
#include <iostream>
#include <string>
#include <sstream>

// util p/ JSON simples
static std::string json_escape(const std::string& s) {
    return SharedMemory::json_escape(s); // reusa do header
}

static void print_json(const std::string& s) {
    std::cout << s << std::endl; // 1 linha (front lê por linha)
}

static void log_kv(const std::string& type, const std::string& msg) {
    std::ostringstream os;
    os << "{\"source\":\"SHM\",\"type\":\"" << type
        << "\",\"message\":\"" << json_escape(msg) << "\"}";
    print_json(os.str());
}

int main(int argc, char** argv) {
    SharedMemory shm(1025); // mesmo tamanho que você já usava (~1 KB)
    if (!shm.initialize()) {
        log_kv("error", "Falha ao inicializar arquivo mapeado");
        return 1;
    }

    std::string cmd = (argc >= 2 ? argv[1] : "status");

    if (cmd == "write") {
        std::string payload;
        if (argc >= 3) {
            // junta argv[2..]
            for (int i = 2; i < argc; ++i) { if (i > 2) payload += ' '; payload += argv[i]; }
        }
        else {
            payload = "Hello IPC";
        }

        bool ok = shm.write_data(payload);
        std::ostringstream os;
        os << "{"
            << "\"ok\":" << (ok ? "true" : "false") << ","
            << "\"event\":\"write\","
            << "\"data\":\"" << json_escape(payload) << "\","
            << "\"source\":\"SHM\""
            << "}";
        print_json(os.str());
        return ok ? 0 : 1;
    }
    else if (cmd == "read") {
        std::string data = shm.read_data();
        std::ostringstream os;
        os << "{"
            << "\"event\":\"read\","
            << "\"data\":\"" << json_escape(data) << "\","
            << "\"source\":\"SHM\""
            << "}";
        print_json(os.str());
        return 0;
    }
    else if (cmd == "clear") {
        shm.clear_memory();
        print_json("{\"ok\":true,\"event\":\"clear\",\"source\":\"SHM\"}");
        return 0;
    }
    else { // status
        std::string st = shm.get_status_json();
        // anexa "source" para o front manter a tag
        std::ostringstream os;
        if (!st.empty() && st.back() == '}') {
            st.pop_back();
            os << st << ",\"source\":\"SHM\"}";
        }
        else {
            os << "{\"source\":\"SHM\"}";
        }
        print_json(os.str());
        return 0;
    }
}
