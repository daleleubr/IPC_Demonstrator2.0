#ifndef SHARED_MEMORY_HPP
#define SHARED_MEMORY_HPP

#include <windows.h>
#include <string>

class SharedMemory {
public:
    // size = capacidade do buffer em bytes (inclui o '\0' de segurança)
    explicit SharedMemory(size_t size = 1024);
    ~SharedMemory();

    bool initialize();                 // abre/cria o arquivo e o mapeamento
    bool write_data(const std::string& data);
    std::string read_data();           // leitura não bloqueante (lê o que estiver salvo)
    void clear_memory();
    std::string get_status_json() const;

    // não copiável
    SharedMemory(const SharedMemory&) = delete;
    SharedMemory& operator=(const SharedMemory&) = delete;

private:
    // caminho completo do arquivo backing (…\ipc_shm.bin)
    std::wstring get_file_path() const;

    size_t  shm_size_;
    HANDLE  h_file_ = INVALID_HANDLE_VALUE;
    HANDLE  h_map_ = nullptr;
    char* view_ = nullptr;
    bool    initialized_ = false;

    // util
    static std::string json_escape(const std::string& s);
};

#endif // SHARED_MEMORY_HPP
