// Pipes.cpp : Este arquivo contém a função 'main'. A execução do programa começa e termina ali.
//

#include <iostream>    // Para entrada/saída (cout, cin)
#include <string>      // Para usar strings do C++
#include <Windows.h>   // API do Windows para pipes
#include <stdio.h>     // Para funções de pipe e entrada e saida
#include <tchar.h>     // Para lidar com caracteres Unicode/ANSI
#define BUFSIZE 512  // (constante)Tamanho do buffer para leitura/escrita

int main()
{
    printf("Testando criacao de pipe...\n");

    HANDLE hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    // Teste de criação do pipe
    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {    
        printf("Falha ao criar pipe! Erro: %d\n", GetLastError());
        return 1;
    }
    printf("Pipe criado com sucesso!\n");

    // Teste de execução do pipe, comandos de escrita e leitura
    printf("Escrevendo no pipe...\n");
    DWORD bytesEscritos; // Variável para armazenar o número de bytes escritos
	char mensagem[] = "Ola pipe!"; // Mensagem a ser escrita no pipe
	BOOL success = WriteFile(hWritePipe, mensagem, strlen(mensagem) + 1, &bytesEscritos, NULL); // +1 para incluir o terminador nulo?
	
    // Verifica se a escrita foi bem-sucedida
    if (!success) {
        printf("Falha ao escrever! Erro: %d\n", GetLastError());
        return 1;
    }
	printf("Mensagem escrita: %s (%d bytes)\n", mensagem, bytesEscritos); // Exibe a mensagem e o número de bytes escritos

    
    printf("Lendo do pipe...\n");
	char buffer[100]; // Buffer para armazenar a mensagem lida
	DWORD bytesLidos; // Variável para armazenar o número de bytes lidos
    success = ReadFile(hReadPipe, buffer, sizeof(buffer), &bytesLidos, NULL);

    if (!success) {
        printf("Falha ao ler! Erro: %d\n", GetLastError());
    }
    else {
        buffer[bytesLidos] = '\0'; // Adiciona terminador nulo
        printf("Mensagem lida: %s (%d bytes)\n", buffer, bytesLidos);
    }
    // IMPORTANTE: Feche os handles para vazar recursos!
    CloseHandle(hReadPipe);
    CloseHandle(hWritePipe);
}



