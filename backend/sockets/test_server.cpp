#include <gtest/gtest.h>
#include <thread>
#include <future>
#include "common.h"

class ServerTest : public ::testing::Test {
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

TEST_F(ServerTest, SocketCreation) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    EXPECT_NE(sock, INVALID_SOCKET);
    if (sock != INVALID_SOCKET) {
        closesocket(sock);
    }
}

TEST_F(ServerTest, BindToPort) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(sock, INVALID_SOCKET);

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    int bind_result = bind(sock, (sockaddr*)&server_addr, sizeof(server_addr));
    EXPECT_NE(bind_result, SOCKET_ERROR);

    closesocket(sock);
}