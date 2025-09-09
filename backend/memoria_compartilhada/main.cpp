#include "shared_memory.hpp"
#include <iostream>
#include <string>
#include <sstream>

// json_escape local — não depende da classe
static std::string json_escape(const std::string& s) {
    std::string out; out.reserve(s.size() + 16);
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
            if (c < 0x20) { char b[7]; std::snprintf(b, sizeof(b), "\\u%04x", c); out += b; }
            else out += (char)c;
        }
    }
    return out;
}

static inline void print_json(const std::string& line) {
    std::cout << line << std::endl; // 1 linha (o front lê por linha)
}

static void log_kv(const char* type, const std::string& msg) {
    std::ostringstream os;
    os << "{\"source\":\"SHM\",\"type\":\"" << type
        << "\",\"message\":\"" << json_escape(msg) << "\"}";
    print_json(os.str());
}

int main(int argc, char** argv) {
    SharedMemory shm(1025);
    if (!shm.initialize()) {
        log_kv("error", "Falha ao inicializar arquivo mapeado");
        return 1;
    }

    const std::string cmd = (argc >= 2 ? argv[1] : "status");

    if (cmd == "write") {
        std::string payload;
        if (argc >= 3) {
            for (int i = 2; i < argc; ++i) { if (i > 2) payload += ' '; payload += argv[i]; }
        }
        else {
            payload = "Hello IPC";
        }
        const bool ok = shm.write_data(payload);
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

    if (cmd == "read") {
        const std::string data = shm.read_data();
        std::ostringstream os;
        os << "{"
            << "\"event\":\"read\","
            << "\"data\":\"" << json_escape(data) << "\","
            << "\"source\":\"SHM\""
            << "}";
        print_json(os.str());
        return 0;
    }

    if (cmd == "clear") {
        shm.clear_memory();
        print_json("{\"ok\":true,\"event\":\"clear\",\"source\":\"SHM\"}");
        return 0;
    }

    // status
    const std::string st = shm.get_status_json();   // já é um JSON completo
    // embrulha como campo "data" sem mexer no conteúdo:
    std::ostringstream os;
    os << "{\"source\":\"SHM\",\"type\":\"status\",\"data\":" << st << "}";
    print_json(os.str());
    return 0;
}
