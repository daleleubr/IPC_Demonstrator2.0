import tkinter as tk
from tkinter import ttk, scrolledtext
import subprocess
import threading
import queue
import json
import os
from pathlib import Path

# =========================
# CONFIG: caminhos fixos
# Ajuste só aqui se necessário
# =========================
HERE = Path(__file__).resolve().parent

# Pipes (pelo print da sua árvore)
EXE_PIPES_PAI   = (HERE / "frontend" / "Pipes" / "Pipes_final" / "Pipes_Final_Pai"   / "Pipes_Final_Pai.exe").resolve()
EXE_PIPES_FILHO = (HERE / "frontend" / "Pipes" / "Pipes_final" / "Pipes_Final_FIlho" / "Pipes_Final_FIlho.exe").resolve()  # atenção ao "FIlho"

# Sockets (compile seus .cpp em .exe com esses nomes)
EXE_SOCK_SERVER = (HERE / "backend" / "sockets" / "server.exe").resolve()
EXE_SOCK_CLIENT = (HERE / "backend" / "sockets" / "cliente.exe").resolve()

# Memória Compartilhada (placeholder)
EXE_SHM         = (HERE / "backend" / "memoria_compartilhada" / "shared_memory.exe").resolve()

# =========================
# App
# =========================
IPC_LIST = ["Pipes", "Sockets", "Memória Compartilhada"]

class FrontIPC(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("IPC – Frontend de Demonstração")
        self.geometry("980x620")

        # processos
        self.proc_pai = None
        self.proc_filho = None
        self.proc_sock_srv = None
        self.proc_sock_cli = None
        self.proc_shm = None

        self.log_q = queue.Queue()

        # topo: seletor
        top = ttk.Frame(self, padding=(10, 10, 10, 0))
        top.pack(fill=tk.X)
        ttk.Label(top, text="Selecionar IPC:").pack(side=tk.LEFT)
        self.sel = ttk.Combobox(top, values=IPC_LIST, state="readonly", width=28)
        self.sel.set("Pipes")
        self.sel.pack(side=tk.LEFT, padx=8)
        self.sel.bind("<<ComboboxSelected>>", self._on_sel)

        # container de seções
        self.container = ttk.Frame(self, padding=10)
        self.container.pack(fill=tk.X)

        self.sec_pipes   = ttk.LabelFrame(self.container, text="Pipes", padding=10)
        self.sec_sockets = ttk.LabelFrame(self.container, text="Sockets", padding=10)
        self.sec_shm     = ttk.LabelFrame(self.container, text="Memória Compartilhada", padding=10)

        self._build_pipes(self.sec_pipes)
        self._build_sockets(self.sec_sockets)
        self._build_shm(self.sec_shm)

        self._show_section("Pipes")

        # log
        lf = ttk.LabelFrame(self, text="Log de Comunicação", padding=10)
        lf.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        self.log = scrolledtext.ScrolledText(lf, state="disabled", wrap=tk.WORD)
        self.log.pack(fill=tk.BOTH, expand=True)
        # tags de cor
        self.log.tag_config("SYS", foreground="gray50")
        self.log.tag_config("PIPES-PAI", foreground="purple")
        self.log.tag_config("PIPES-FILHO", foreground="medium orchid")
        self.log.tag_config("SOCKET-SERVER", foreground="blue")
        self.log.tag_config("SOCKET-CLIENT", foreground="dark green")
        self.log.tag_config("SHM", foreground="brown")
        self.log.tag_config("ERR", foreground="red")

        # loop de drenagem do log
        self.after(100, self._drain_log)

    # ---------- seções ----------
    def _build_pipes(self, parent):
        r1 = ttk.Frame(parent); r1.pack(fill=tk.X, pady=4)
        ttk.Label(r1, text="Mensagem (se seus binários aceitarem argv[1]):").pack(side=tk.LEFT)
        self.msg_pipes = ttk.Entry(r1); self.msg_pipes.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=8)
        self.msg_pipes.insert(0, "OiPipes")

        r2 = ttk.Frame(parent); r2.pack(fill=tk.X, pady=6)
        ttk.Button(r2, text="Iniciar PAI", command=self.start_pipes_pai).pack(side=tk.LEFT)
        ttk.Button(r2, text="Iniciar FILHO", command=self.start_pipes_filho).pack(side=tk.LEFT, padx=6)
        ttk.Button(r2, text="Parar PAI/FILHO", command=self.stop_pipes).pack(side=tk.LEFT, padx=6)

    def _build_sockets(self, parent):
        r1 = ttk.Frame(parent); r1.pack(fill=tk.X, pady=4)
        ttk.Label(r1, text="Mensagem do cliente").pack(side=tk.LEFT)
        self.msg_sock = ttk.Entry(r1); self.msg_sock.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=8)
        self.msg_sock.insert(0, "OiSockets")

        r2 = ttk.Frame(parent); r2.pack(fill=tk.X, pady=6)
        ttk.Button(r2, text="Iniciar Servidor", command=self.start_sock_server).pack(side=tk.LEFT)
        ttk.Button(r2, text="Iniciar Cliente", command=self.start_sock_client).pack(side=tk.LEFT, padx=6)
        ttk.Button(r2, text="Parar Servidor/Cliente", command=self.stop_sockets).pack(side=tk.LEFT, padx=6)

    def _build_shm(self, parent):
        r1 = ttk.Frame(parent); r1.pack(fill=tk.X, pady=4)
        ttk.Label(r1, text="Mensagem (se suportado):").pack(side=tk.LEFT)
        self.msg_shm = ttk.Entry(r1); self.msg_shm.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=8)
        self.msg_shm.insert(0, "OiMemoriaCompartilhada")

        r2 = ttk.Frame(parent); r2.pack(fill=tk.X, pady=6)
        ttk.Button(r2, text="Executar SHM", command=self.start_shm).pack(side=tk.LEFT)
        ttk.Button(r2, text="Parar SHM", command=self.stop_shm).pack(side=tk.LEFT, padx=6)

        ttk.Label(parent, text="Observação: implemente seu app SHM para imprimir JSON por linha (type/message) ou texto cru.\n"
                               "O log interpreta JSON automaticamente; caso contrário, mostra como [RAW].").pack(anchor="w", pady=(8,0))

    def _on_sel(self, _e=None):
        self._show_section(self.sel.get())

    def _show_section(self, name: str):
        for f in (self.sec_pipes, self.sec_sockets, self.sec_shm):
            f.pack_forget()
        if name.startswith("Pipes"):
            self.sec_pipes.pack(fill=tk.X)
        elif name.startswith("Sockets"):
            self.sec_sockets.pack(fill=tk.X)
        else:
            self.sec_shm.pack(fill=tk.X)

    # ---------- logging ----------
    def _log(self, text: str, tag=None):
        self.log.configure(state="normal")
        if tag:
            self.log.insert(tk.END, text + "\n", tag)
        else:
            self.log.insert(tk.END, text + "\n")
        self.log.configure(state="disabled")
        self.log.see(tk.END)

    def _enqueue_json(self, d: dict):
        try:
            self.log_q.put_nowait(json.dumps(d))
        except Exception as e:
            self.log_q.put_nowait(json.dumps({"source":"SYS","type":"error","message":f"Falha ao enfileirar: {e}"}))

    def _inject_and_enqueue(self, raw_line: str, role: str):
        raw_line = raw_line.rstrip("\n")
        if not raw_line:
            return
        try:
            obj = json.loads(raw_line)
            if "source" not in obj:
                obj["source"] = role
            self._enqueue_json(obj)
        except json.JSONDecodeError:
            self._enqueue_json({"source": role, "type": "raw", "message": raw_line})

    def _drain_log(self):
        try:
            while not self.log_q.empty():
                line = self.log_q.get_nowait()
                try:
                    d = json.loads(line)
                    source = d.get("source", "SYS")
                    typ    = d.get("type", "log")
                    msg    = d.get("message", "")
                    tag = {
                        "SYS":"SYS",
                        "PIPES-PAI":"PIPES-PAI",
                        "PIPES-FILHO":"PIPES-FILHO",
                        "SOCKET-SERVER":"SOCKET-SERVER",
                        "SOCKET-CLIENT":"SOCKET-CLIENT",
                        "SHM":"SHM"
                    }.get(source, "SYS")
                    if typ.lower() == "error":
                        tag = "ERR"
                    self._log(f"[{source} - {typ}]: {msg}", tag)
                except Exception:
                    self._log(str(line), "SYS")
        finally:
            self.after(100, self._drain_log)

    # ---------- runner helpers ----------
    def _spawn(self, args, role):
        exe = str(args[0])
        cwd = os.getcwd()
        if not os.path.exists(exe):
            self._log(f"[SYS - error]: Executável não encontrado: {exe}", "ERR")
            self._log(f"[SYS - info]: CWD={cwd}", "SYS")
            return None

        self._log(f"[SYS - run]: {args}", "SYS")
        self._log(f"[SYS - info]: CWD={cwd}", "SYS")

        try:
            proc = subprocess.Popen(
                args,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,  # junta stderr -> evita deadlock e não perde erro
                text=True,
                bufsize=1
            )
        except Exception as e:
            self._log(f"[SYS - error]: Falha ao iniciar {role}: {e}", "ERR")
            return None

        # aviso se ficar "mudo" (sem \n/flush)
        def _warn_if_silent():
            if proc.poll() is None:
                self._log(f"[SYS - warn]: {role} ainda sem saída. "
                          f"Garanta que o backend imprime linhas com '\\n' (std::endl).", "SYS")
        self.after(2000, _warn_if_silent)

        t = threading.Thread(target=self._pump, args=(proc, role), daemon=True)
        t.start()
        return proc

    def _pump(self, proc, role):
        try:
            for line in iter(proc.stdout.readline, ''):
                self._inject_and_enqueue(line, role)
            proc.stdout.close()
            rc = proc.wait()
            self._enqueue_json({"source":"SYS","type":"log","message":f"{role} finalizado (rc={rc})."})
        except Exception as e:
            self._enqueue_json({"source":"SYS","type":"error","message":f"{role} erro: {e}"})

    # ---------- Pipes ----------
    def start_pipes_pai(self):
        args = [str(EXE_PIPES_PAI)]
        msg = self.msg_pipes.get().strip()
        if msg:
            args.append(msg)  # só se seu PAI aceitar argv[1]
        self.proc_pai = self._spawn(args, "PIPES-PAI")

    def start_pipes_filho(self):
        args = [str(EXE_PIPES_FILHO)]
        msg = self.msg_pipes.get().strip()
        if msg:
            args.append(msg)  # só se seu FILHO aceitar argv[1]
        self.proc_filho = self._spawn(args, "PIPES-FILHO")

    def stop_pipes(self):
        stopped = []
        for name, p in (("PAI", self.proc_pai), ("FILHO", self.proc_filho)):
            if p and p.poll() is None:
                try:
                    p.terminate()
                    stopped.append(name)
                except Exception as e:
                    self._enqueue_json({"source":"SYS","type":"error","message":f"Falha ao parar {name}: {e}"})
        if stopped:
            self._enqueue_json({"source":"SYS","type":"log","message":"Parados: " + ", ".join(stopped)})
        else:
            self._log("[SYS - log]: Nenhum processo Pipes em execução.", "SYS")

    # ---------- Sockets ----------
    def start_sock_server(self):
        self.proc_sock_srv = self._spawn([str(EXE_SOCK_SERVER)], "SOCKET-SERVER")

    def start_sock_client(self):
        # se seu cliente aceitar argv[1], passe a mensagem:
        # msg = self.msg_sock.get().strip()
        # args = [str(EXE_SOCK_CLIENT)] + ([msg] if msg else [])
        args = [str(EXE_SOCK_CLIENT)]
        self.proc_sock_cli = self._spawn(args, "SOCKET-CLIENT")

    def stop_sockets(self):
        stopped = []
        for name, p in (("Servidor", self.proc_sock_srv), ("Cliente", self.proc_sock_cli)):
            if p and p.poll() is None:
                try:
                    p.terminate()
                    stopped.append(name)
                except Exception as e:
                    self._enqueue_json({"source":"SYS","type":"error","message":f"Falha ao parar {name}: {e}"})
        if stopped:
            self._enqueue_json({"source":"SYS","type":"log","message":"Parados: " + ", ".join(stopped)})
        else:
            self._log("[SYS - log]: Nenhum processo Sockets em execução.", "SYS")

    # ---------- SHM ----------
    def start_shm(self):
        args = [str(EXE_SHM)]
        msg = self.msg_shm.get().strip()
        if msg:
            args.append(msg)  # se o seu exe aceitar argv[1]
        self.proc_shm = self._spawn(args, "SHM")

    def stop_shm(self):
        if self.proc_shm and self.proc_shm.poll() is None:
            try:
                self.proc_shm.terminate()
                self._enqueue_json({"source":"SYS","type":"log","message":"SHM parado."})
            except Exception as e:
                self._enqueue_json({"source":"SYS","type":"error","message":f"Falha ao parar SHM: {e}"})
        else:
            self._log("[SYS - log]: Nenhum processo SHM em execução.", "SYS")

# run
if __name__ == "__main__":
    FrontIPC().mainloop()
