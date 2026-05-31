import json
import serial
import serial.tools.list_ports
import threading
import queue
import tkinter as tk
from tkinter import ttk, messagebox, scrolledtext


# =============================================================================
# SerialManager — maneja la conexión y lectura en hilo separado
# =============================================================================
class SerialManager:
    def __init__(self, message_queue):
        self.queue = message_queue
        self.connection = None
        self.running = False
        self._thread = None

    def list_ports(self):
        return [p.device for p in serial.tools.list_ports.comports()]

    def connect(self, port, baudrate=115200):
        self.connection = serial.Serial(port, baudrate, timeout=1)
        self.running = True
        self._thread = threading.Thread(target=self._read_loop, daemon=True)
        self._thread.start()

    def disconnect(self):
        self.running = False
        if self.connection and self.connection.is_open:
            self.connection.close()

    def send(self, command):
        """Envía comando ASCII terminado en \\n al ESP32."""
        if self.connection and self.connection.is_open:
            self.connection.write(f"{command}\n".encode())

    def _read_loop(self):
        while self.running:
            try:
                line = self.connection.readline().decode("utf-8", errors="replace").strip()
                if line:
                    self.queue.put(line)
            except Exception as e:
                self.queue.put(f"[ERROR] Serial: {e}")
                break


# =============================================================================
# InventoryMap — cuadrícula 4 columnas × 3 filas
# =============================================================================
class InventoryMap(tk.Frame):
    COLUMNS = ["A", "B", "C", "D"]
    ROWS    = [1, 2, 3]

    COLOR_EMPTY    = "#d0f0c0"
    COLOR_OCCUPIED = "#f4a460"
    COLOR_UNKNOWN  = "#e0e0e0"

    def __init__(self, parent):
        super().__init__(parent, padx=10, pady=10)
        self.cells = {}
        self._build()

    def _build(self):
        tk.Label(self, text="INVENTARIO", font=("Arial", 10, "bold")).grid(
            row=0, column=0, columnspan=len(self.COLUMNS), pady=(0, 6))

        for c, col in enumerate(self.COLUMNS):
            tk.Label(self, text=col, font=("Arial", 9, "bold"), width=10).grid(row=1, column=c)

        for r, row in enumerate(self.ROWS):
            tk.Label(self, text=str(row), font=("Arial", 9, "bold")).grid(
                row=r + 2, column=len(self.COLUMNS), padx=(4, 0))
            for c, col in enumerate(self.COLUMNS):
                cell_id = f"{col}{row}"
                frame = tk.Frame(self, width=80, height=55,
                                 bg=self.COLOR_UNKNOWN, relief="solid", bd=1)
                frame.grid(row=r + 2, column=c, padx=3, pady=3)
                frame.grid_propagate(False)

                lbl_id = tk.Label(frame, text=cell_id, font=("Arial", 8, "bold"),
                                  bg=self.COLOR_UNKNOWN)
                lbl_id.place(x=4, y=4)

                lbl_status = tk.Label(frame, text="?", font=("Arial", 7),
                                      bg=self.COLOR_UNKNOWN)
                lbl_status.place(relx=0.5, rely=0.62, anchor="center")

                self.cells[cell_id] = {
                    "frame": frame, "lbl_id": lbl_id,
                    "lbl_status": lbl_status, "state": "unknown"
                }

    def update_cell(self, cell_id, weight_g):
        """Actualiza una celda con el peso recibido del firmware (0 = vacía)."""
        if cell_id not in self.cells:
            return
        cell = self.cells[cell_id]

        if weight_g > 0:
            color = self.COLOR_OCCUPIED
            text  = f"{int(weight_g)}g"
            state = "occupied"
        else:
            color = self.COLOR_EMPTY
            text  = "VACÍA"
            state = "empty"

        cell["frame"].config(bg=color)
        cell["lbl_id"].config(bg=color)
        cell["lbl_status"].config(bg=color, text=text)
        cell["state"] = state

    def reset(self):
        for cell_id in self.cells:
            cell = self.cells[cell_id]
            for widget in ("frame", "lbl_id", "lbl_status"):
                cell[widget].config(bg=self.COLOR_UNKNOWN)
            cell["lbl_status"].config(text="?")
            cell["state"] = "unknown"


# =============================================================================
# LogPanel — visor de logs con colores por nivel
# =============================================================================
class LogPanel(tk.Frame):
    COLORS = {
        "INFO":  "#4fc3f7",
        "WARN":  "#ffb74d",
        "ERROR": "#ef5350",
    }

    def __init__(self, parent):
        super().__init__(parent, padx=10, pady=10)
        self._build()

    def _build(self):
        header = tk.Frame(self)
        header.pack(fill="x")
        tk.Label(header, text="LOG DE EVENTOS", font=("Arial", 10, "bold")).pack(side="left")
        tk.Button(header, text="Limpiar", command=self._clear).pack(side="right")

        self.text = scrolledtext.ScrolledText(
            self, width=72, height=6,
            font=("Courier", 8), state="disabled",
            bg="#1e1e1e", fg="#ffffff"
        )
        self.text.pack(fill="both", expand=True, pady=(6, 0))

        for level, color in self.COLORS.items():
            self.text.tag_config(level, foreground=color)
        self.text.tag_config("DEFAULT", foreground="#cccccc")
        self.text.tag_config("JSON_OK",  foreground="#81c784")
        self.text.tag_config("JSON_ERR", foreground="#ef5350")

    def append(self, line):
        self.text.config(state="normal")

        tag = "DEFAULT"
        for level in self.COLORS:
            if f"][{level}" in line or f"][{level} " in line:
                tag = level
                break
        # JSON de respuesta
        if '"status":"OK"' in line:
            tag = "JSON_OK"
        elif '"status":"ERROR"' in line:
            tag = "JSON_ERR"

        self.text.insert("end", line + "\n", tag)
        self.text.see("end")
        self.text.config(state="disabled")

    def _clear(self):
        self.text.config(state="normal")
        self.text.delete("1.0", "end")
        self.text.config(state="disabled")


# =============================================================================
# CommandPanel — envío de comandos STORE / RETRIEVE / SWAP / RESET
# =============================================================================
class CommandPanel(tk.Frame):
    COLUMNS = ["A", "B", "C", "D"]
    ROWS    = ["1", "2", "3"]

    def __init__(self, parent, send_callback):
        super().__init__(parent, padx=10, pady=8)
        self.send = send_callback
        self._build()

    def _build(self):
        tk.Label(self, text="COMANDOS", font=("Arial", 10, "bold")).grid(
            row=0, column=0, columnspan=4, pady=(0, 6), sticky="w")

        # --- STORE ---
        tk.Label(self, text="STORE celda:").grid(row=1, column=0, sticky="w")
        self.store_col = ttk.Combobox(self, values=self.COLUMNS, width=4, state="readonly")
        self.store_col.current(0)
        self.store_col.grid(row=1, column=1, padx=2)
        self.store_row = ttk.Combobox(self, values=self.ROWS, width=4, state="readonly")
        self.store_row.current(0)
        self.store_row.grid(row=1, column=2, padx=2)
        tk.Button(self, text="STORE", width=8, bg="#4caf50", fg="white",
                  command=self._send_store).grid(row=1, column=3, padx=(6, 0))

        # --- RETRIEVE ---
        tk.Label(self, text="RETRIEVE celda:").grid(row=2, column=0, sticky="w", pady=(4, 0))
        self.retr_col = ttk.Combobox(self, values=self.COLUMNS, width=4, state="readonly")
        self.retr_col.current(0)
        self.retr_col.grid(row=2, column=1, padx=2, pady=(4, 0))
        self.retr_row = ttk.Combobox(self, values=self.ROWS, width=4, state="readonly")
        self.retr_row.current(0)
        self.retr_row.grid(row=2, column=2, padx=2, pady=(4, 0))
        tk.Button(self, text="RETRIEVE", width=8, bg="#2196f3", fg="white",
                  command=self._send_retrieve).grid(row=2, column=3, padx=(6, 0), pady=(4, 0))

        # --- SWAP ---
        tk.Label(self, text="SWAP de:").grid(row=3, column=0, sticky="w", pady=(4, 0))
        self.swap_col1 = ttk.Combobox(self, values=self.COLUMNS, width=4, state="readonly")
        self.swap_col1.current(0)
        self.swap_col1.grid(row=3, column=1, padx=2, pady=(4, 0))
        self.swap_row1 = ttk.Combobox(self, values=self.ROWS, width=4, state="readonly")
        self.swap_row1.current(0)
        self.swap_row1.grid(row=3, column=2, padx=2, pady=(4, 0))

        tk.Label(self, text="a:").grid(row=4, column=0, sticky="w")
        self.swap_col2 = ttk.Combobox(self, values=self.COLUMNS, width=4, state="readonly")
        self.swap_col2.current(1)
        self.swap_col2.grid(row=4, column=1, padx=2)
        self.swap_row2 = ttk.Combobox(self, values=self.ROWS, width=4, state="readonly")
        self.swap_row2.current(0)
        self.swap_row2.grid(row=4, column=2, padx=2)
        tk.Button(self, text="SWAP", width=8, bg="#ff9800", fg="white",
                  command=self._send_swap).grid(row=3, column=3, rowspan=2,
                                                padx=(6, 0), pady=(4, 0), sticky="ns")

        # --- RESET ---
        tk.Button(self, text="RESET", width=10, bg="#e53935", fg="white",
                  command=self._send_reset).grid(row=5, column=0, columnspan=4,
                                                  pady=(10, 0), sticky="w")

    def _send_store(self):
        cell = f"{self.store_col.get()}{self.store_row.get()}"
        self.send(f"STORE:{cell}")

    def _send_retrieve(self):
        cell = f"{self.retr_col.get()}{self.retr_row.get()}"
        self.send(f"RETRIEVE:{cell}")

    def _send_swap(self):
        c1 = f"{self.swap_col1.get()}{self.swap_row1.get()}"
        c2 = f"{self.swap_col2.get()}{self.swap_row2.get()}"
        if c1 == c2:
            messagebox.showwarning("SWAP inválido", "Las dos celdas deben ser diferentes.")
            return
        self.send(f"SWAP:{c1}:{c2}")

    def _send_reset(self):
        self.send("RESET")


# =============================================================================
# ASRSApp — aplicación principal
# =============================================================================
class ASRSApp:
    STATE_COLORS = {
        "IDLE":      "green",
        "OPERANDO":  "blue",
        "HOMING":    "purple",
        "DEGRADED":  "orange",
        "FAULT":     "red",
        "E-STOP":    "red",
        "BOOT":      "gray",
    }

    def __init__(self, root):
        self.root = root
        self.root.title("AS/RS — Panel de Control")
        self.root.geometry("520x700")
        self.root.resizable(True, True)

        self.queue  = queue.Queue()
        self.serial = SerialManager(self.queue)

        self._build_ui()
        self._poll_queue()

    def _build_ui(self):
        # --- Barra de conexión ---
        top = tk.Frame(self.root, padx=10, pady=8)
        top.pack(fill="x")

        tk.Label(top, text="Puerto:").pack(side="left")
        self.port_var = tk.StringVar()
        self.port_combo = ttk.Combobox(top, textvariable=self.port_var,
                                        width=12, state="readonly")
        self.port_combo.pack(side="left", padx=(4, 8))
        tk.Button(top, text="↻", command=self._refresh_ports).pack(side="left", padx=(0, 8))
        self.btn_connect = tk.Button(top, text="Conectar", width=10,
                                      command=self._toggle_connect)
        self.btn_connect.pack(side="left")

        tk.Label(top, text="Estado:").pack(side="left", padx=(20, 4))
        self.state_var   = tk.StringVar(value="DESCONECTADO")
        self.state_label = tk.Label(top, textvariable=self.state_var,
                                     font=("Courier", 10, "bold"), fg="gray", width=14)
        self.state_label.pack(side="left")

        # --- Cuerpo principal: mapa + comandos lado a lado ---
        body = tk.Frame(self.root, padx=10)
        body.pack(fill="both")

        left = tk.Frame(body)
        left.pack(side="left", anchor="n")

        self.inventory_map = InventoryMap(left)
        self.inventory_map.pack()

        ttk.Separator(left, orient="horizontal").pack(fill="x", pady=4)

        self.cmd_panel = CommandPanel(left, self._send_command)
        self.cmd_panel.pack(anchor="w")

        # --- Log abajo ---
        ttk.Separator(self.root, orient="horizontal").pack(fill="x", padx=10, pady=4)
        self.log_panel = LogPanel(self.root)
        self.log_panel.pack(fill="both", padx=0, pady=(0, 6))

        self._refresh_ports()

    def _refresh_ports(self):
        ports = self.serial.list_ports()
        self.port_combo["values"] = ports
        if ports:
            self.port_combo.current(0)

    def _toggle_connect(self):
        if self.serial.connection and self.serial.connection.is_open:
            self.serial.disconnect()
            self.btn_connect.config(text="Conectar")
            self._set_state("DESCONECTADO", "gray")
            self.inventory_map.reset()
        else:
            port = self.port_var.get()
            if not port:
                messagebox.showerror("Error", "Selecciona un puerto primero.")
                return
            try:
                self.serial.connect(port)
                self.btn_connect.config(text="Desconectar")
                self._set_state("CONECTADO", "green")
            except Exception as e:
                messagebox.showerror("Error de conexión", str(e))

    def _set_state(self, text, color="black"):
        self.state_var.set(text)
        self.state_label.config(fg=color)

    def _send_command(self, cmd):
        """Envía comando y lo registra en el log."""
        self.serial.send(cmd)
        self.log_panel.append(f"[TX] {cmd}")

    # -------------------------------------------------------------------------
    # Procesamiento de mensajes JSON del firmware
    # -------------------------------------------------------------------------
    def _process_message(self, line):
        """Intenta parsear JSON del firmware y actualizar la UI."""
        try:
            data = json.loads(line)
        except json.JSONDecodeError:
            # Línea de log normal del firmware ([ts][LEVEL][MODULE] msg)
            return

        # {"event":"STATE","state":"IDLE","inventory":{"A1":245,"B2":0,...}}
        if data.get("event") == "STATE":
            state = data.get("state", "")
            color = self.STATE_COLORS.get(state, "black")
            self._set_state(state, color)

            inv = data.get("inventory", {})
            for cell_id, weight in inv.items():
                self.inventory_map.update_cell(cell_id, float(weight))

        # {"status":"OK","op":"STORE","cell":"A1","weight":245.3}
        elif data.get("status") == "OK":
            op     = data.get("op", "")
            cell   = data.get("cell", "")
            weight = float(data.get("weight", 0))
            if op == "STORE":
                self.inventory_map.update_cell(cell, weight)
            elif op == "RETRIEVE":
                self.inventory_map.update_cell(cell, 0)

        # {"status":"ERROR","code":"ERR_CELL_OCCUPIED","cell":"A1"}
        elif data.get("status") == "ERROR":
            code = data.get("code", "ERROR")
            cell = data.get("cell", "")
            messagebox.showwarning("Error del sistema", f"{code}\nCelda: {cell}")

    def _poll_queue(self):
        try:
            while True:
                message = self.queue.get_nowait()
                self.log_panel.append(message)
                self._process_message(message)
        except queue.Empty:
            pass
        self.root.after(100, self._poll_queue)


if __name__ == "__main__":
    root = tk.Tk()
    app  = ASRSApp(root)
    root.mainloop()
