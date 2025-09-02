import tkinter ##Gerar interface gráfica
from tkinter import ttk, scrolledtext #Importa componentes extras do Tkinter, como botões, frames, etc...
import subprocess #Permite executar programas externos (como o backend em C++)
import threading #Criar e gerenciar Threads
import json #Trabalhar com dados no formato JSON
import queue #Fornece uma fila para comunicação entre Threads
import os #Interagir com o sistema operacional

class IPC_Frontend:
    def __init__(self, root):
        self.root = root
        self.root.title("Visualizador de Comunicação IPC")
        self.root.geometry("600x400")