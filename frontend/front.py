import tkinter as tk
from tkinter import ttk, scrolledtext
import subprocess
import threading
import json
import queue
import os

class IPC_Frontend:
    def __init__(self, root):
        self.root = root
        self.root.title("Visualizador de Comunicação IPC")
        self.root.geometry("600x400")

        # Fila para comunicação thread-safe entre o leitor de output e a GUI
        self.log_queue = queue.Queue()

        # Frame principal
        main_frame = ttk.Frame(root, padding="10")
        main_frame.pack(fill=tk.BOTH, expand=True)

        # Seção de Controles
        controls_frame = ttk.LabelFrame(main_frame, text="Controles do Pipe", padding="10")
        controls_frame.pack(fill=tk.X, pady=5)

        ttk.Label(controls_frame, text="Mensagem:").pack(side=tk.LEFT, padx=(0, 5))
        self.message_entry = ttk.Entry(controls_frame, width=40)
        self.message_entry.pack(side=tk.LEFT, expand=True, fill=tk.X)
        self.message_entry.insert(0, "OlaMundoDoPipe")

        self.send_button = ttk.Button(controls_frame, text="Enviar via Pipe", command=self.start_pipe_process)
        self.send_button.pack(side=tk.LEFT, padx=(5, 0))

        # Seção de Log
        log_frame = ttk.LabelFrame(main_frame, text="Log de Comunicação", padding="10")
        log_frame.pack(fill=tk.BOTH, expand=True)

        self.log_text = scrolledtext.ScrolledText(log_frame, wrap=tk.WORD, state='disabled')
        self.log_text.pack(fill=tk.BOTH, expand=True)

        # Inicia o processamento da fila de logs
        self.root.after(100, self.process_log_queue)

    def log(self, message):
        """ Adiciona uma mensagem na área de log da GUI """
        self.log_text.configure(state='normal')
        self.log_text.insert(tk.END, message + "\n")
        self.log_text.configure(state='disabled')
        self.log_text.see(tk.END)

    def start_pipe_process(self):
        """ Inicia a execução do backend C++ em uma nova thread """
        message = self.message_entry.get()
        if not message:
            self.log("!! ERRO: A mensagem não pode estar vazia.")
            return
        
        # O caminho para o executável C++. Ajuste se necessário.
        # Assume que a pasta 'backend' está no mesmo nível que a 'frontend'
        backend_path = os.path.join("..", "backend", "pipes", "pipes_app")
        
        if not os.path.exists(backend_path):
            self.log(f"!! ERRO: Executável não encontrado em '{backend_path}'")
            self.log("!! Certifique-se de que o backend C++ foi compilado.")
            return

        self.log("--- Iniciando processo C++ para Pipes ---")
        self.send_button.config(state='disabled')
        
        # Usa uma thread para executar o processo e ler sua saída,
        # para não travar a interface gráfica 
        thread = threading.Thread(target=self.run_and_read_output, args=(backend_path, message))
        thread.start()

    def run_and_read_output(self, command, message):
        """ Executa o processo e coloca suas saídas na fila """
        try:
            # Inicia o processo [cite: 62]
            process = subprocess.Popen(
                [command, message],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True, # Decodifica a saída como texto
                bufsize=1  # Leitura linha a linha
            )

            # Captura a saída (stdout) em tempo real [cite: 62]
            for line in iter(process.stdout.readline, ''):
                self.log_queue.put(line)

            process.stdout.close()
            process.wait()
            
            # Adiciona uma mensagem final na fila
            self.log_queue.put(json.dumps({"source": "system", "type":"log", "message": "--- Processo C++ finalizado ---"}))

        except Exception as e:
            self.log_queue.put(json.dumps({"source": "frontend", "type":"error", "message": f"Erro ao executar processo: {e}"}))

    def process_log_queue(self):
        """ Processa mensagens da fila e atualiza a GUI """
        try:
            while not self.log_queue.empty():
                line = self.log_queue.get_nowait()
                try:
                    # Interpreta o JSON recebido do backend [cite: 62]
                    data = json.loads(line)
                    source = data.get("source", "N/A").upper()
                    msg_type = data.get("type", "N/A")
                    message = data.get("message", "")
                    
                    # Formata a saída para ser mais legível
                    formatted_log = f"[{source} - {msg_type}]: {message}"
                    self.log(formatted_log)
                except json.JSONDecodeError:
                    # Se não for um JSON válido, apenas loga a linha crua
                    self.log(f"[RAW]: {line.strip()}")

        finally:
            # Re-habilita o botão se a fila estiver vazia (processo terminou)
            if self.log_queue.empty():
                 self.send_button.config(state='normal')
            self.root.after(100, self.process_log_queue)


if __name__ == "__main__":
    root = tk.Tk()
    app = IPC_Frontend(root)
    root.mainloop()