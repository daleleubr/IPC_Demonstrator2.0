#include <gtest/gtest.h>
#include "common.h"

class ClientTest : public ::testing::Test {
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

TEST_F(ClientTest, SocketCreation) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    EXPECT_NE(sock, INVALID_SOCKET);
    if (sock != INVALID_SOCKET) {
        closesocket(sock);
    }
}

TEST_F(ClientTest, InvalidConnection) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(sock, INVALID_SOCKET);

    sockaddr_in hint{};
    hint.sin_family = AF_INET;
    hint.sin_port = htons(9999); // Porta inválida
    hint.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Deve falhar ao conectar com porta inválida
    EXPECT_EQ(connect(sock, (sockaddr*)&hint, sizeof(hint)), SOCKET_ERROR);

    closesocket(sock);
}