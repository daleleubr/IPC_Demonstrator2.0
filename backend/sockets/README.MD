**COMMON.H** - Arquivo de cabeçalho comum para o projeto de sockets em C++ no Windows.

#include <windows.h>      // API principal do Windows  - Arquivos, janelas, processos
#include <winsock2.h>     // API de sockets do Windows - Permite comunicação entre si, rede
#include <iostream>       // Entrada/saída padrão
#include <string>         // Manipulação de strings   
#include <sstream>        // String streams			   - Ex: Concatenar, comparar strings
#include <locale.h>       // Configurações de localidade - Formatação local, data, moeda, acentos.
#pragma once              // Garante que seja incluído apenas uma vez
#define _WINSOCK_DEPRECATED_NO_WARNINGS // Suprime avisos de funções obsoletas
#define WIN32_LEAN_AND_MEAN// Reduz o tempo de compilação

// Função para JSON - Cria uma mensagem JSON padronizada
// Inicializa Winsock - Inicia a biblioteca de rede do Windowns, carrega a DLL do Winsock
						versão 2.2, retorna true ou false
						Se erro Usa a própria função make_json() para reportar o erro

// Finaliza Winsock - Limpa os recursos de rede, libera a memória e fecha conexões
// Constante da porta - Define a porta padrão para conexões de rede, porta 54000

Resumo: Inicia a biblioteca de rede, formata msg JSON, Limpa os recursos de rede.

**CLIENTE.cpp** - Implementação do cliente de sockets para comunicação TCP/IP no Windows.

if (!init_winsock()) return 1; - Inicia Winsock
SOCKET sock - Cria o socket, IPv4, TCP e protocolo padrão (0)
sockaddr_in hint - Define o endereço do servidor
if (connect(sock - Conecta ao servidor
**  Lembrando que o IP e a porta são fixos**
RESUMO: Conecta ao servidor,inicia biblioteca de rede, operações registradas no JSON,
		envia e recebe mensagens, fecha o socket.


**SERVER.cpp** - Implementação do cliente de sockets para comunicação TCP/IP no Windows.
f (!init_winsock()) return 1; - Inicializa a biblioteca Winsock

SOCKET server_socket - Cria socket
sockaddr_in server_addr{};					 Define estrutura do endereço do servidor
server_addr.sin_family = AF_INET;			- Faz a configuração para IPv4
server_addr.sin_port = htons(PORT);			- Define porta 54000
server_addr.sin_addr.s_addr = INADDR_ANY;   - Aceita conexões de qualquer interface
if (bind(server_socket,						- Associa socket ao endereço
if (listen(server_sock						- Coloca socket em modo escuta
SOCKET client_socket = accept(server_soc	- Aceita conexão de cliente
int bytes_received = recv(clie				- Recebe dados do cliente
send(client_socket, response.c_str(),		- Envia resposta ao cliente
closesocket(client_socket);					- Fecha socket do cliente
closesocket(server_socket);					- Fecha socket do servidor
cleanup_winsock();							- Limpa recursos da Winsock


