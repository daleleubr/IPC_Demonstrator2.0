#include "common.h"

// Cliente Socket
// HENRIQUE - Sistema de comunicação via sockets

int main(int argc, char** argv) {            
    setlocale(LC_ALL, "Portuguese_Brazil.1252");

    if (!init_winsock()) return 1;

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << make_json("error", "Não foi possível criar socket cliente") << std::endl;
        cleanup_winsock();
        return 1;
    }

    sockaddr_in hint{};
    hint.sin_family = AF_INET;
    hint.sin_port = htons(PORT);
    hint.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
        std::cerr << make_json("error", "Não foi possível conectar ao servidor") << std::endl;
        closesocket(sock);
        cleanup_winsock();
        return 1;
    }

    std::cout << make_json("status", "Conectado ao servidor") << std::endl;

    // ---------- MENSAGEM: usa argv se veio do front ----------
    auto join_args = [](int argc, char** argv, int start)->std::string {
        std::string out;
        for (int i = start; i < argc; ++i) { if (i > start) out += ' '; out += argv[i]; }
        return out;
        };
    std::string msg = (argc >= 2) ? join_args(argc, argv, 1)
        : "Olá servidor, aqui é o cliente!";
    // ---------------------------------------------------------

    int sendResult = send(sock, msg.c_str(), (int)msg.size() + 1, 0);
    if (sendResult == SOCKET_ERROR) {
        std::cerr << make_json("error", "Falha no envio") << std::endl;
    }
    else {
        std::cout << make_json("sent", msg) << std::endl;
    }

    char buf[4096]{};
    int bytesReceived = recv(sock, buf, 4096, 0);
    if (bytesReceived > 0) {
        std::string reply(buf, 0, bytesReceived);
        std::cout << make_json("received", reply) << std::endl;
    }
    else if (bytesReceived == 0) {
        std::cout << make_json("status", "Servidor desconectado") << std::endl;
    }
    else {
        std::cerr << make_json("error", "Falha ao receber") << std::endl;
    }

    closesocket(sock);
    cleanup_winsock();
    std::cout << make_json("status", "Cliente finalizado") << std::endl;
    return 0;
}