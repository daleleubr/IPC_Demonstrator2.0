#include "common.h"

// Cliente Socket
// HENRIQUE - Sistema de comunicação via sockets

int main() {
	setlocale(LC_ALL, "Portuguese_Brazil.1252");  //Corrigir caracteres especiais

	//Inicia Biblioteca Winsock
    if (!init_winsock()) return 1;

	//Cria o socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << make_json("error", "Não foi possível criar socket cliente") << std::endl;
        cleanup_winsock();
        return 1;
    }

    //configura o servidor
    sockaddr_in hint{};
    hint.sin_family = AF_INET;
    hint.sin_port = htons(PORT);
    hint.sin_addr.s_addr = inet_addr("127.0.0.1");

	//Conecta ao servidor
    if (connect(sock, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
        std::cerr << make_json("error", "Não foi possível conectar ao servidor") << std::endl;
        closesocket(sock);
        cleanup_winsock();
        return 1;
    }

    //MSG 
    std::cout << make_json("status", "Conectado ao servidor") << std::endl;

    std::string msg = "Olá servidor, aqui é o cliente!";
    int sendResult = send(sock, msg.c_str(), msg.size() + 1, 0);
    if (sendResult == SOCKET_ERROR) {
        std::cerr << make_json("error", "Falha no envio") << std::endl;
    }
    else {
        std::cout << make_json("sent", msg) << std::endl;
    }

	//Recebe a resposta do servidor (**Até 4096 bytes**)
    char buf[4096];
    ZeroMemory(buf, 4096);

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

	//Fecha o socket e limpa recursos
	closesocket(sock); //finaliza o socket
    cleanup_winsock(); //Finaliza o sistema

    std::cout << make_json("status", "Cliente finalizado") << std::endl;
    return 0;
}