#include <gtest/gtest.h>
#include <thread>
#include <future>
#include "common.h"

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        if (!init_winsock()) {
            GTEST_SKIP() << "Winsock initialization failed";
        }
    }

    void TearDown() override {
        cleanup_winsock();
    }
};

void runServer() {
    // Implementação simplificada do servidor para testes
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) return;

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        closesocket(server_socket);
        return;
    }

    listen(server_socket, 1);

    sockaddr_in client_addr{};
    int client_size = sizeof(client_addr);
    SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_size);

    if (client_socket != INVALID_SOCKET) {
        char buffer[1024];
        int bytes = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes > 0) {
            send(client_socket, "response", 9, 0);
        }
        closesocket(client_socket);
    }

    closesocket(server_socket);
}

TEST_F(IntegrationTest, BasicCommunication) {
    // Inicia servidor em thread separada
    std::thread server_thread(runServer);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Testa cliente
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(sock, INVALID_SOCKET);

    sockaddr_in hint{};
    hint.sin_family = AF_INET;
    hint.sin_port = htons(PORT);
    hint.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (sockaddr*)&hint, sizeof(hint)) != SOCKET_ERROR) {
        send(sock, "test", 5, 0);

        char buffer[1024];
        int bytes = recv(sock, buffer, sizeof(buffer), 0);
        EXPECT_GT(bytes, 0);
    }

    closesocket(sock);
    server_thread.join();
}