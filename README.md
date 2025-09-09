# IPC_Demonstrator2.0
1) Projeto didático de Comunicação Entre Processos (IPC) em Windows com três técnicas:
    - Sockets (TCP/Winsock) — servidor/cliente
    - Pipes (Named Pipes) — processos Pai/Filho
    - Memória Compartilhada (SHM) — arquivo mapeado + utilitários
    O projeto inclui um frontend em Python/Tkinter que:
    - Oferece um seletor do tipo de IPC (Pipes / Sockets / Memória Compartilhada);
    - Possui uma caixa de mensagem (“Mensagem para teste”) usada pelos backends;
    - Mostra um log em tempo real interpretando JSON por linha.

2) Estrutura do repositório:
    IPC_Demonstrator2.0/
    ├─ frontend.py                         # GUI Tkinter (seletor, mensagem e log)
    ├─ backend/
    │  ├─ sockets/
    │  │  ├─ server.cpp
    │  │  ├─ cliente.cpp
    │  │  ├─ server.exe                 # (onde o front procura)
    │  │  └─ cliente.exe                # (onde o front procura)
    │  ├─ pipes/
    │  │  └─ Pipes_final/
    │  │     ├─ Pipes_Final_Pai/        # projetos VS (binários saem em Debug/Release)
    │  │     ├─ Pipes_Final_FIlho/
    │  │     ├─ Pipes_Final_Pai.exe     # (copiar aqui após compilar)
    │  │     └─ Pipes_Final_FIlho.exe   # (copiar aqui após compilar)
    │  └─ memoria_compartilhada/
    │     ├─ shared_memory.hpp
    │     ├─ shared_memory.cpp
    │     ├─ main.cpp                   # wrapper CLI (status/read/write/clear)
    │     └─ shm_app.exe                # (onde o front procura)
    └─ README.md

3) Requisitos:
    - Windows 10/11 x64
    - Python 3.9+ (Tkinter já vem no instalador oficial)
    - Visual Studio 2022 (Desktop development with C++) ou MSVC Build Tools
    - Permissão no Firewall para os executáveis de Sockets (se necessário)

4) Compilando os Backends
    Abra o x64 Native Tools Command Prompt for VS 2022 (Iniciar → Visual Studio 2022 → Developer Command Prompts).
    1) Sockets (Winsock)
        cd /d "<repo>\backend\sockets"
        cl /nologo /EHsc /W4 /std:c++20 server.cpp    /Fe:server.exe
        cl /nologo /EHsc /W4 /std:c++20 cliente.cpp   /Fe:cliente.exe
    2) Caminhos esperados pelo front:
        backend\sockets\server.exe
        backend\sockets\cliente.exe
    3) Pipes (Named Pipes – Pai/Filho)
        Abra as soluções/projetos no VS:
        backend\pipes\Pipes_final\Pipes_Final_Pai\
        backend\pipes\Pipes_final\Pipes_Final_FIlho\
        Compile em x64 / Release (ou Debug). Depois copie os executáveis para a pasta Pipes_final (onde o front procura):
        copy /y "backend\pipes\Pipes_final\Pipes_Final_Pai\x64\Release\Pipes_Final_Pai.exe"     "backend\pipes\Pipes_final\Pipes_Final_Pai.exe"
        copy /y "backend\pipes\Pipes_final\Pipes_Final_FIlho\x64\Release\Pipes_Final_FIlho.exe" "backend\pipes\Pipes_final\Pipes_Final_FIlho.exe"
        Atenção à grafia: Pipes_Final_FIlho (há um “I” maiúsculo no nome do projeto).
        Fluxo no front:
        Iniciar PAI → 2) Iniciar FILHO.
        A mensagem digitada é passada via argv e usada no envio/resposta pelo pipe.
    4) Memória Compartilhada (SHM)
    cd /d "<repo>\backend\memoria_compartilhada"
    cl /nologo /EHsc /W4 /std:c++20 main.cpp shared_memory.cpp /Fe:shm_app.exe
    Caminho esperado pelo front:
    backend\memoria_compartilhada\shm_app.exe
    Comandos suportados (o front dispara automaticamente):
    shm_app.exe status
    shm_app.exe read
    shm_app.exe clear
    shm_app.exe write <mensagem>
    Implementação usa arquivo mapeado (file-backed), então o conteúdo persiste entre execuções.
    O front “lê por linha”: cada operação imprime exatamente 1 linha JSON terminada por \n.

5) 🕹️ Como usar (fluxos)
    Sockets
    Clique Iniciar Servidor.
    Digite uma mensagem.
    Clique Iniciar Cliente (a mensagem segue para o servidor).
    Veja status, sent, received no log.

    Pipes
    Clique Iniciar PAI (servidor do pipe).
    Clique Iniciar FILHO (cliente do pipe).
    A mensagem digitada é enviada/ecoada via pipe (veja os logs de Pai/Filho).

    Memória Compartilhada
    Write: grava o texto digitado.
    Read: lê o conteúdo atual.
    Status: mostra JSON com caminho do arquivo, tamanho, conteúdo, etc.
    Clear: zera o buffer.

6) 🧾 Formato de Log
    Os backends imprimem uma linha JSON por evento (terminada com \n). Exemplos:
    {"source":"SOCKET-SERVER","type":"status","message":"Servidor aguardando conexões na porta 54000"}
    {"source":"SOCKET-CLIENT","type":"sent","message":"Hello IPC"}
    {"source":"PIPES-PAI","type":"received","message":"..."}
    {"source":"SHM","type":"status","data":{"shared_memory":{"file":"...","size":1025,"content":"Hello IPC"}}}
    Se a linha não for JSON, o front mostra como [RAW].

7) ❗ Solução de Problemas
    “Executável não encontrado” no log
    → Falta copiar os .exe para os caminhos esperados (ver árvore acima).

    Sem “finalizado (rc=0)” após um botão
    → O backend não imprimiu \n no fim da linha ou não retornou do main.
    Garanta std::endl/'\n' e return 0; ao final.

    .exe.recipe
    → Não é executável. Ignore/adicione ao .gitignore.

    Firewall bloqueando Sockets
    → Permita server.exe/cliente.exe no Firewall.

    MSVC/cl não encontrado
    → Abrir o x64 Native Tools Command Prompt for VS 2022.

    Compilou mas não acho o .exe
    → Procure com dir /s *.exe ou copie de x64\Release\ para os caminhos esperados.

8) 📝 Licença
    Projeto para fins educacionais. Ajuste esta seção conforme necessário.