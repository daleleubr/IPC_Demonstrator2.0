#include "shared_memory.hpp"
#include <json/json.h>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>

// Nome/tamanho fixos do segmento usados pelo demo
static const char* kName = "ipc_demo";
static const size_t kSize = 4096;

// Emite um único JSON (com \n) e encerra com rc
static void print_json(const Json::Value& v, int rc = 0) {
    Json::StreamWriterBuilder w;
    std::string s = Json::writeString(w, v);
    if (s.empty() || s.back() != '\n') s.push_back('\n');
    std::cout << s;
    std::cout.flush();
    std::exit(rc);
}

static bool parse_json_string(const std::string& s, Json::Value& out, std::string& err) {
    Json::CharReaderBuilder r;
    std::istringstream iss(s);
    return Json::parseFromStream(r, iss, &out, &err);
}

int main(int argc, char** argv) {
    Json::Value msg;

    if (argc < 2) {
        msg["ok"] = false;
        msg["event"] = "usage";
        msg["error"] = "use: shm_app <status|read|clear>";
        print_json(msg, 1);
    }

    std::string cmd = argv[1];

    SharedMemory shm{ kName, kSize };
    if (!shm.initialize()) {
        msg["ok"] = false;
        msg["event"] = cmd;
        msg["error"] = "failed to initialize shared memory/semaphores";
        print_json(msg, 2);
    }

    if (cmd == "status") {
        std::string raw = shm.get_status_json();
        Json::Value status;
        std::string err;
        if (!parse_json_string(raw, status, err)) {
            msg["ok"] = false;
            msg["event"] = "status";
            msg["error"] = "invalid status json";
            msg["detail"] = err;
            print_json(msg, 3);
        }
        status["ok"] = true;
        status["event"] = "status";
        print_json(status, 0);
    }
    else if (cmd == "read") {
        std::string content = shm.read_data(); // bloqueia até haver dado (semáforo)
        msg["ok"] = (content.rfind("ERROR:", 0) != 0);
        msg["event"] = "read";
        msg["content"] = content;

        // snapshot de status (opcional no log)
        std::string raw = shm.get_status_json();
        Json::Value st; std::string err;
        if (parse_json_string(raw, st, err)) msg["status_snapshot"] = st;

        print_json(msg, msg["ok"].asBool() ? 0 : 4);
    }
    else if (cmd == "clear") {
        shm.clear_memory();
        msg["ok"] = true;
        msg["event"] = "clear";

        std::string raw = shm.get_status_json();
        Json::Value st; std::string err;
        if (parse_json_string(raw, st, err)) msg["status_snapshot"] = st;

        print_json(msg, 0);
    }
    else {
        msg["ok"] = false;
        msg["event"] = cmd;
        msg["error"] = "unknown command (expected: status|read|clear)";
        print_json(msg, 5);
    }
}
