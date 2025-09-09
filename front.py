import tkinter as tk
from tkinter import ttk, scrolledtext
import subprocess, threading, queue, json, os, platform
from pathlib import Path

# ========= Descobrir raiz do projeto e caminhos =========
HERE = Path(__file__).resolve().parent

def find_project_root(start: Path) -> Path:
    p = start
    for _ in range(6):  # sobe até 6 níveis
        if (p / "backend").exists():
            return p
        p = p.parent
    return start

PROJECT_ROOT = find_project_root(HERE)

# Ajuste os nomes se os .exe estiverem em Debug/Release etc.
EXE_PIPES_PAI   = (PROJECT_ROOT / "backend" / "pipes" / "Pipes_final" / "Pipes_Final_Pai"   / "Debug" / "Pipes_Final_Pai.exe").resolve()
EXE_PIPES_FILHO = (PROJECT_ROOT / "backend" / "pipes" / "Pipes_final" / "Pipes_Final_Pai"   / "Debug" / "Pipes_Final_FIlho.exe").resolve()
EXE_SOCK_SERVER = (PROJECT_ROOT / "backend"  / "sockets" / "server.exe").resolve()
EXE_SOCK_CLIENT = (PROJECT_ROOT / "backend"  / "sockets" / "cliente.exe").resolve()
EXE_SHM         = (PROJECT_ROOT / "backend"  / "memoria_compartilhada" / "shm_app.exe").resolve()  # versão Win32

IPC_LIST = ["Pipes", "Sockets", "Memória Compartilhada"]

class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("IPC – Frontend (sem input de mensagens)")
        self.geometry("920x560")

        self.log_q = queue.Queue()
        self._seen_output = {}

        # processos que podem ficar vivos (Pipes/Sockets). SHM aqui é execução por comando.
        self.proc_pai = None
        self.proc_filho = None
        self.proc_sock_srv = None
        self.proc_sock_cli = None

        # topo – seletor
        top = ttk.Frame(self, padding=(10,10,10,0))
        top.pack(fill=tk.X)
        ttk.Label(top, text="Selecionar IPC:").pack(side=tk.LEFT)
        self.sel = ttk.Combobox(top, values=IPC_LIST, state="readonly", width=28)
        self.sel.set("Sockets")
        self.sel.pack(side=tk.LEFT, padx=8)
        self.sel.bind("<<ComboboxSelected>>", lambda _e: self._show(self.sel.get()))

        # container de seções
        self.body = ttk.Frame(self, padding=10); self.body.pack(fill=tk.X)

        self.sec_pipes   = ttk.LabelFrame(self.body, text="Pipes", padding=10)
        self.sec_sockets = ttk.LabelFrame(self.body, text="Sockets", padding=10)
        self.sec_shm     = ttk.LabelFrame(self.body, text="Memória Compartilhada", padding=10)

        self._build_pipes(self.sec_pipes)
        self._build_sockets(self.sec_sockets)
        self._build_shm(self.sec_shm)

        self._show(self.sel.get())

        # Log
        lf = ttk.LabelFrame(self, text="Log de Comunicação", padding=10)
        lf.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        self.log = scrolledtext.ScrolledText(lf, state="disabled", wrap=tk.WORD)
        self.log.pack(fill=tk.BOTH, expand=True)

        # Cores
        self.log.tag_config("SYS", foreground="gray50")
        self.log.tag_config("PIPES-PAI", foreground="purple")
        self.log.tag_config("PIPES-FILHO", foreground="medium orchid")
        self.log.tag_config("SOCKET-SERVER", foreground="blue")
        self.log.tag_config("SOCKET-CLIENT", foreground="dark green")
        self.log.tag_config("SHM", foreground="brown")
        self.log.tag_config("ERR", foreground="red")

        # loop do log
        self.after(100, self._drain)

    # --------- UI ---------
    def _build_pipes(self, parent):
        r = ttk.Frame(parent); r.pack(fill=tk.X)
        ttk.Button(r, text="Iniciar PAI", command=self.start_pipes_pai).pack(side=tk.LEFT)
        ttk.Button(r, text="Iniciar FILHO", command=self.start_pipes_filho).pack(side=tk.LEFT, padx=6)
        ttk.Button(r, text="Parar PAI/FILHO", command=self.stop_pipes).pack(side=tk.LEFT, padx=6)

    def _build_sockets(self, parent):
        r = ttk.Frame(parent); r.pack(fill=tk.X)
        ttk.Button(r, text="Iniciar Servidor", command=self.start_sock_server).pack(side=tk.LEFT)
        ttk.Button(r, text="Iniciar Cliente", command=self.start_sock_client).pack(side=tk.LEFT, padx=6)
        ttk.Button(r, text="Parar Servidor/Cliente", command=self.stop_sockets).pack(side=tk.LEFT, padx=6)

    def _build_shm(self, parent):
        info = ("Observação: mensagens são definidas no próprio programa.\n"
                "Use os botões abaixo apenas para acionar ações sem payload.")
        ttk.Label(parent, text=info).pack(anchor="w", pady=(0,8))
        r = ttk.Frame(parent); r.pack(fill=tk.X)
        ttk.Button(r, text="Status", command=self.shm_status).pack(side=tk.LEFT)
        ttk.Button(r, text="Read", command=self.shm_read).pack(side=tk.LEFT, padx=6)
        ttk.Button(r, text="Clear", command=self.shm_clear).pack(side=tk.LEFT, padx=6)
        ttk.Button(r, text="Write (Hello)", command=self.shm_write_test).pack(side=tk.LEFT, padx=6)

    def _show(self, which: str):
        for f in (self.sec_pipes, self.sec_sockets, self.sec_shm):
            f.pack_forget()
        if which.startswith("Pipes"):
            self.sec_pipes.pack(fill=tk.X)
        elif which.startswith("Sockets"):
            self.sec_sockets.pack(fill=tk.X)
        else:
            self.sec_shm.pack(fill=tk.X)

    # --------- Log helpers ---------
    def _log(self, text, tag=None):
        self.log.configure(state="normal")
        self.log.insert(tk.END, text + "\n", tag)
        self.log.configure(state="disabled")
        self.log.see(tk.END)

    def _enqueue(self, d: dict):
        self.log_q.put_nowait(json.dumps(d))

    def _inject_or_raw(self, raw, role):
        raw = raw.rstrip("\n")
        if not raw: return
        try:
            obj = json.loads(raw)
            if "source" not in obj:
                obj["source"] = role
            self._enqueue(obj)
        except json.JSONDecodeError:
            self._enqueue({"source": role, "type":"raw", "message": raw})

    def _drain(self):
        try:
            while not self.log_q.empty():
                j = json.loads(self.log_q.get_nowait())
                source = j.get("source","SYS")
                typ    = j.get("type","log")
                msg    = j.get("message")
                if not msg:
                    # imprime o JSON inteiro se não houver 'message'
                    msg = json.dumps(j, ensure_ascii=False)
                tag = {
                    "SYS":"SYS","PIPES-PAI":"PIPES-PAI","PIPES-FILHO":"PIPES-FILHO",
                    "SOCKET-SERVER":"SOCKET-SERVER","SOCKET-CLIENT":"SOCKET-CLIENT",
                    "SHM":"SHM"
                }.get(source, "SYS")
                if typ.lower()=="error": tag="ERR"
                self._log(f"[{source} - {typ}]: {msg}", tag)
        finally:
            self.after(100, self._drain)

    # --------- Exec helpers ---------
    def _spawn(self, args, role):
        exe = str(args[0])
        cwd = os.getcwd()
        if not os.path.exists(exe):
            self._log(f"[SYS - error]: Executável não encontrado: {exe}", "ERR")
            self._log(f"[SYS - info]: PROJECT_ROOT={PROJECT_ROOT}", "SYS")
            self._log(f"[SYS - info]: CWD={cwd}", "SYS")
            return None

        self._log(f"[SYS - run]: {args}", "SYS")
        self._log(f"[SYS - info]: CWD={cwd}", "SYS")

        try:
            proc = subprocess.Popen(
                args,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                bufsize=1
            )
        except Exception as e:
            self._log(f"[SYS - error]: Falha ao iniciar {role}: {e}", "ERR")
            return None

        self._seen_output[role] = False

        def warn_if_silent():
            if proc.poll() is None and not self._seen_output.get(role, False):
                self._log(f"[SYS - warn]: {role} ainda sem saída. "
                          f"Garanta que o backend imprime linhas com '\\n' (std::endl).", "SYS")
        self.after(2000, warn_if_silent)

        t = threading.Thread(target=self._pump, args=(proc, role), daemon=True)
        t.start()
        return proc

    def _pump(self, proc, role):
        try:
            for line in iter(proc.stdout.readline, ''):
                if not self._seen_output.get(role, False):
                    self._seen_output[role] = True
                self._inject_or_raw(line, role)
            proc.stdout.close()
            rc = proc.wait()
            self._enqueue({"source":"SYS","type":"log","message":f"{role} finalizado (rc={rc})."})
        except Exception as e:
            self._enqueue({"source":"SYS","type":"error","message":f"{role} erro: {e}"})

    # --------- Pipes ---------
    def start_pipes_pai(self):   self.proc_pai   = self._spawn([str(EXE_PIPES_PAI)],   "PIPES-PAI")
    def start_pipes_filho(self): self.proc_filho = self._spawn([str(EXE_PIPES_FILHO)], "PIPES-FILHO")

    def stop_pipes(self):
        stopped=[]
        for name,p in (("PAI",self.proc_pai),("FILHO",self.proc_filho)):
            if p and p.poll() is None:
                try: p.terminate(); stopped.append(name)
                except Exception as e: self._enqueue({"source":"SYS","type":"error","message":f"Falha parar {name}: {e}"})
        if stopped: self._enqueue({"source":"SYS","type":"log","message":"Parados: "+", ".join(stopped)})
        else: self._log("[SYS - log]: Nenhum processo Pipes em execução.", "SYS")

    # --------- Sockets ---------
    def start_sock_server(self): self.proc_sock_srv = self._spawn([str(EXE_SOCK_SERVER)], "SOCKET-SERVER")
    def start_sock_client(self): self.proc_sock_cli  = self._spawn([str(EXE_SOCK_CLIENT)], "SOCKET-CLIENT")

    def stop_sockets(self):
        stopped=[]
        for name,p in (("Servidor",self.proc_sock_srv),("Cliente",self.proc_sock_cli)):
            if p and p.poll() is None:
                try: p.terminate(); stopped.append(name)
                except Exception as e: self._enqueue({"source":"SYS","type":"error","message":f"Falha parar {name}: {e}"})
        if stopped: self._enqueue({"source":"SYS","type":"log","message":"Parados: "+", ".join(stopped)})
        else: self._log("[SYS - log]: Nenhum processo Sockets em execução.", "SYS")

    # --------- SHM (sem payload) ---------
    def shm_status(self): self._spawn([str(EXE_SHM), "status"], "SHM")
    def shm_read(self):   self._spawn([str(EXE_SHM), "read"],   "SHM")
    def shm_clear(self):  self._spawn([str(EXE_SHM), "clear"],  "SHM")
    def shm_write_test(self):
        self._spawn([str(EXE_SHM), "write", "Hello IPC"], "SHM")

if __name__ == "__main__":
    App().mainloop()
