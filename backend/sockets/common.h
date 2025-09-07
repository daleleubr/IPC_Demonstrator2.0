#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <iostream>
#include <string>
#include <sstream>
#include <locale.h> 

#pragma comment(lib, "ws2_32.lib")

// Função para JSON
inline std::string make_json(const std::string& type, const std::string& message) {
    std::stringstream ss;
    ss << "{\"type\":\"" << type << "\", \"message\":\"" << message << "\"}";
    return ss.str();
}

// Inicializa Winsock
inline bool init_winsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << make_json("error", "Falha ao iniciar Winsock") << std::endl;
        return false;
    }
    return true;
}

// Finaliza Winsock
inline void cleanup_winsock() {
    WSACleanup();
}

// Constante da porta
const int PORT = 54000;