#!/usr/bin/env python3
"""
dsPIC33AK Audio Demo - Control GUI

Real-time audio DSP control panel for the dsPIC33AK Audio Demo project.
Communicates with the board via UART (115200 8-N-1).

Features:
    - EQ band sliders (5-band parametric)
    - Bass/Treble adjustment
    - Volume control
    - Echo on/off with delay setting
    - Noise gate on/off
    - Input source selection
    - Demo mode selection
    - Status display

Requirements:
    pip install pyserial tkinter

Usage:
    python audio_control_gui.py [COM_PORT]
    Example: python audio_control_gui.py COM3
"""

import sys
import tkinter as tk
from tkinter import ttk, messagebox
import serial
import serial.tools.list_ports
import threading
import time


class AudioControlGUI:
    """Main GUI application for dsPIC33AK Audio Demo control."""

    def __init__(self, root):
        self.root = root
        self.root.title("dsPIC33AK Audio DSP Control")
        self.root.geometry("800x650")
        self.root.resizable(True, True)

        self.serial_port = None
        self.connected = False
        self.rx_thread = None
        self.running = True

        self._build_gui()

    def _build_gui(self):
        """Build the complete GUI layout."""

        # ---- Connection Frame ----
        conn_frame = ttk.LabelFrame(self.root, text="Connection", padding=5)
        conn_frame.pack(fill=tk.X, padx=5, pady=5)

        ttk.Label(conn_frame, text="Port:").pack(side=tk.LEFT, padx=2)
        self.port_var = tk.StringVar()
        self.port_combo = ttk.Combobox(conn_frame, textvariable=self.port_var,
                                        width=15, state="readonly")
        self.port_combo.pack(side=tk.LEFT, padx=2)
        self._refresh_ports()

        ttk.Button(conn_frame, text="Refresh",
                   command=self._refresh_ports).pack(side=tk.LEFT, padx=2)
        self.connect_btn = ttk.Button(conn_frame, text="Connect",
                                       command=self._toggle_connection)
        self.connect_btn.pack(side=tk.LEFT, padx=5)
        self.status_label = ttk.Label(conn_frame, text="Disconnected",
                                       foreground="red")
        self.status_label.pack(side=tk.LEFT, padx=10)

        # ---- Mode / Input Frame ----
        mode_frame = ttk.LabelFrame(self.root, text="Mode & Input", padding=5)
        mode_frame.pack(fill=tk.X, padx=5, pady=2)

        ttk.Label(mode_frame, text="Demo Mode:").pack(side=tk.LEFT, padx=5)
        self.mode_var = tk.StringVar(value="1")
        for text, val in [("Loopback", "1"), ("Test Tone", "2"),
                          ("Visualizer", "3")]:
            ttk.Radiobutton(mode_frame, text=text, variable=self.mode_var,
                           value=val, command=self._send_mode
                           ).pack(side=tk.LEFT, padx=3)

        ttk.Separator(mode_frame, orient=tk.VERTICAL).pack(
            side=tk.LEFT, fill=tk.Y, padx=10)

        ttk.Label(mode_frame, text="Input:").pack(side=tk.LEFT, padx=5)
        self.input_var = tk.StringVar(value="codec")
        for text, val in [("Codec", "codec"), ("Mic", "mic"),
                          ("Tone", "tone")]:
            ttk.Radiobutton(mode_frame, text=text, variable=self.input_var,
                           value=val, command=self._send_input
                           ).pack(side=tk.LEFT, padx=3)

        # ---- Volume Frame ----
        vol_frame = ttk.LabelFrame(self.root, text="Volume", padding=5)
        vol_frame.pack(fill=tk.X, padx=5, pady=2)

        self.vol_var = tk.IntVar(value=200)
        self.vol_label = ttk.Label(vol_frame, text="200")
        ttk.Label(vol_frame, text="Volume:").pack(side=tk.LEFT, padx=5)
        ttk.Scale(vol_frame, from_=0, to=255, variable=self.vol_var,
                  orient=tk.HORIZONTAL, command=self._on_volume_change
                  ).pack(side=tk.LEFT, fill=tk.X, expand=True, padx=5)
        self.vol_label.pack(side=tk.LEFT, padx=5)

        # ---- EQ Frame ----
        eq_frame = ttk.LabelFrame(self.root, text="5-Band Equalizer", padding=5)
        eq_frame.pack(fill=tk.X, padx=5, pady=2)

        self.eq_vars = []
        self.eq_labels = []
        band_names = ["60 Hz", "230 Hz", "910 Hz", "4 kHz", "14 kHz"]

        for i, name in enumerate(band_names):
            col_frame = ttk.Frame(eq_frame)
            col_frame.pack(side=tk.LEFT, fill=tk.Y, expand=True, padx=5)

            ttk.Label(col_frame, text=name, anchor=tk.CENTER).pack()
            var = tk.IntVar(value=5)
            self.eq_vars.append(var)
            lbl = ttk.Label(col_frame, text="5", width=3, anchor=tk.CENTER)
            self.eq_labels.append(lbl)

            band_idx = i
            ttk.Scale(col_frame, from_=10, to=0, variable=var,
                      orient=tk.VERTICAL, length=150,
                      command=lambda val, b=band_idx: self._on_eq_change(b)
                      ).pack(pady=2)
            lbl.pack()

        # ---- Bass / Treble Frame ----
        bt_frame = ttk.LabelFrame(self.root, text="Bass & Treble", padding=5)
        bt_frame.pack(fill=tk.X, padx=5, pady=2)

        # Bass
        bass_frame = ttk.Frame(bt_frame)
        bass_frame.pack(side=tk.LEFT, fill=tk.X, expand=True)
        ttk.Label(bass_frame, text="Bass:").pack(side=tk.LEFT, padx=5)
        self.bass_var = tk.IntVar(value=5)
        self.bass_label = ttk.Label(bass_frame, text="5")
        ttk.Scale(bass_frame, from_=0, to=10, variable=self.bass_var,
                  orient=tk.HORIZONTAL,
                  command=self._on_bass_change).pack(
                      side=tk.LEFT, fill=tk.X, expand=True, padx=5)
        self.bass_label.pack(side=tk.LEFT, padx=5)

        # Treble
        treb_frame = ttk.Frame(bt_frame)
        treb_frame.pack(side=tk.LEFT, fill=tk.X, expand=True)
        ttk.Label(treb_frame, text="Treble:").pack(side=tk.LEFT, padx=5)
        self.treble_var = tk.IntVar(value=5)
        self.treble_label = ttk.Label(treb_frame, text="5")
        ttk.Scale(treb_frame, from_=0, to=10, variable=self.treble_var,
                  orient=tk.HORIZONTAL,
                  command=self._on_treble_change).pack(
                      side=tk.LEFT, fill=tk.X, expand=True, padx=5)
        self.treble_label.pack(side=tk.LEFT, padx=5)

        # ---- Effects Frame ----
        fx_frame = ttk.LabelFrame(self.root, text="Effects", padding=5)
        fx_frame.pack(fill=tk.X, padx=5, pady=2)

        self.noise_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(fx_frame, text="Noise Gate",
                        variable=self.noise_var,
                        command=self._on_noise_toggle).pack(
                            side=tk.LEFT, padx=10)

        self.echo_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(fx_frame, text="Echo",
                        variable=self.echo_var,
                        command=self._on_echo_toggle).pack(
                            side=tk.LEFT, padx=10)

        self.stream_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(fx_frame, text="Stream Samples",
                        variable=self.stream_var,
                        command=self._on_stream_toggle).pack(
                            side=tk.LEFT, padx=10)

        # Test tone frequency
        ttk.Label(fx_frame, text="Tone Freq (Hz):").pack(side=tk.LEFT, padx=5)
        self.freq_var = tk.StringVar(value="1000")
        freq_entry = ttk.Entry(fx_frame, textvariable=self.freq_var, width=6)
        freq_entry.pack(side=tk.LEFT, padx=2)
        ttk.Button(fx_frame, text="Set",
                   command=self._send_freq).pack(side=tk.LEFT, padx=2)

        # ---- Console Frame ----
        console_frame = ttk.LabelFrame(self.root, text="Console", padding=5)
        console_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        self.console_text = tk.Text(console_frame, height=8, width=80,
                                     state=tk.DISABLED, bg="#1e1e1e",
                                     fg="#00ff00", font=("Consolas", 9))
        scrollbar = ttk.Scrollbar(console_frame,
                                   command=self.console_text.yview)
        self.console_text.configure(yscrollcommand=scrollbar.set)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.console_text.pack(fill=tk.BOTH, expand=True)

        # Manual command entry
        cmd_frame = ttk.Frame(console_frame)
        cmd_frame.pack(fill=tk.X, pady=2)
        ttk.Label(cmd_frame, text=">").pack(side=tk.LEFT)
        self.cmd_entry = ttk.Entry(cmd_frame)
        self.cmd_entry.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=2)
        self.cmd_entry.bind("<Return>", self._send_manual_cmd)

    def _refresh_ports(self):
        """Refresh available COM ports."""
        ports = [p.device for p in serial.tools.list_ports.comports()]
        self.port_combo["values"] = ports
        if ports:
            self.port_combo.current(0)

    def _toggle_connection(self):
        """Connect or disconnect from the serial port."""
        if self.connected:
            self._disconnect()
        else:
            self._connect()

    def _connect(self):
        """Open serial connection."""
        port = self.port_var.get()
        if not port:
            messagebox.showerror("Error", "No port selected")
            return
        try:
            self.serial_port = serial.Serial(port, 115200, timeout=0.1)
            self.connected = True
            self.connect_btn.configure(text="Disconnect")
            self.status_label.configure(text=f"Connected: {port}",
                                        foreground="green")
            self._console_log(f"Connected to {port}")

            # Start RX thread
            self.rx_thread = threading.Thread(target=self._rx_loop,
                                              daemon=True)
            self.rx_thread.start()

            # Request status
            self._send_cmd("status")

        except serial.SerialException as e:
            messagebox.showerror("Connection Error", str(e))

    def _disconnect(self):
        """Close serial connection."""
        self.connected = False
        if self.serial_port:
            self.serial_port.close()
            self.serial_port = None
        self.connect_btn.configure(text="Connect")
        self.status_label.configure(text="Disconnected", foreground="red")
        self._console_log("Disconnected")

    def _send_cmd(self, cmd):
        """Send a command string to the board."""
        if self.serial_port and self.connected:
            try:
                self.serial_port.write(f"{cmd}\r".encode())
                self._console_log(f"> {cmd}")
            except serial.SerialException:
                self._disconnect()

    def _rx_loop(self):
        """Background thread to read serial data."""
        buffer = ""
        while self.connected and self.running:
            try:
                if self.serial_port and self.serial_port.in_waiting:
                    data = self.serial_port.read(
                        self.serial_port.in_waiting).decode(
                            "ascii", errors="replace")
                    buffer += data
                    while "\n" in buffer:
                        line, buffer = buffer.split("\n", 1)
                        line = line.strip()
                        if line:
                            self.root.after(0, self._console_log, line)
                else:
                    time.sleep(0.05)
            except (serial.SerialException, OSError):
                self.root.after(0, self._disconnect)
                break

    def _console_log(self, text):
        """Append text to the console output."""
        self.console_text.configure(state=tk.NORMAL)
        self.console_text.insert(tk.END, text + "\n")
        self.console_text.see(tk.END)
        self.console_text.configure(state=tk.DISABLED)

    # ---- Control Callbacks ----

    def _on_volume_change(self, val):
        v = int(float(val))
        self.vol_label.configure(text=str(v))
        self._send_cmd(f"vol {v}")

    def _on_eq_change(self, band):
        v = self.eq_vars[band].get()
        self.eq_labels[band].configure(text=str(v))
        self._send_cmd(f"eq{band + 1} {v}")

    def _on_bass_change(self, val):
        v = int(float(val))
        self.bass_label.configure(text=str(v))
        self._send_cmd(f"bass {v}")

    def _on_treble_change(self, val):
        v = int(float(val))
        self.treble_label.configure(text=str(v))
        self._send_cmd(f"treble {v}")

    def _on_noise_toggle(self):
        state = "on" if self.noise_var.get() else "off"
        self._send_cmd(f"noise {state}")

    def _on_echo_toggle(self):
        state = "on" if self.echo_var.get() else "off"
        self._send_cmd(f"echo {state}")

    def _on_stream_toggle(self):
        state = "on" if self.stream_var.get() else "off"
        self._send_cmd(f"stream {state}")

    def _send_mode(self):
        self._send_cmd(f"mode {self.mode_var.get()}")

    def _send_input(self):
        self._send_cmd(f"input {self.input_var.get()}")

    def _send_freq(self):
        self._send_cmd(f"freq {self.freq_var.get()}")

    def _send_manual_cmd(self, event):
        cmd = self.cmd_entry.get().strip()
        if cmd:
            self._send_cmd(cmd)
            self.cmd_entry.delete(0, tk.END)

    def on_close(self):
        """Clean shutdown."""
        self.running = False
        self._disconnect()
        self.root.destroy()


def main():
    root = tk.Tk()
    app = AudioControlGUI(root)
    root.protocol("WM_DELETE_WINDOW", app.on_close)

    # Auto-connect if port specified on command line
    if len(sys.argv) > 1:
        app.port_var.set(sys.argv[1])
        app._connect()

    root.mainloop()


if __name__ == "__main__":
    main()
