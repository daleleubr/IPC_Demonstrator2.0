// ConsoleApplication1.cpp : Este arquivo contém a função 'main'. A execução do programa começa e termina ali.
//

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

int main() {
    printf("Sou o processo PAI (PID: %d).\n", GetCurrentProcessId());

    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;
	TCHAR   commandLine[] = TEXT("calc.exe");

    if (!CreateProcess(
        NULL,           // Nome do aplicativo
        commandLine,     // Comando para executar a calculadora
        NULL,           // Atributos de segurança do processo
        NULL,           // Atributos de segurança da thread
        FALSE,          // Herança de handles
        0,              // Flags de criação
        NULL,           // Ambiente
        NULL,           // Diretório atual
        &si,            // STARTUPINFO
        &pi             // PROCESS_INFORMATION
    )) {
        printf("Falha ao criar processo. Erro: %d\n", GetLastError());
        return 1;
    }

    printf("Processo filho criado com sucesso! PID: %d\n", pi.dwProcessId);
    WaitForSingleObject(pi.hProcess, INFINITE); // Espera calculadora fechar
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}

// Executar programa: Ctrl + F5 ou Menu Depurar > Iniciar Sem Depuração
// Depurar programa: F5 ou menu Depurar > Iniciar Depuração

// Dicas para Começar: 
//   1. Use a janela do Gerenciador de Soluções para adicionar/gerenciar arquivos
//   2. Use a janela do Team Explorer para conectar-se ao controle do código-fonte
//   3. Use a janela de Saída para ver mensagens de saída do build e outras mensagens
//   4. Use a janela Lista de Erros para exibir erros
//   5. Ir Para o Projeto > Adicionar Novo Item para criar novos arquivos de código, ou Projeto > Adicionar Item Existente para adicionar arquivos de código existentes ao projeto
//   6. No futuro, para abrir este projeto novamente, vá para Arquivo > Abrir > Projeto e selecione o arquivo. sln
