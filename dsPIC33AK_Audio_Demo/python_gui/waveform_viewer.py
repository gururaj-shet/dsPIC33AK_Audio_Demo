#!/usr/bin/env python3
"""
dsPIC33AK Audio Demo - Waveform Viewer

Real-time audio waveform and spectrum visualization from
dsPIC33AK Audio Demo board via UART.

Protocol:
    Start frame:  0x03 0xFC
    Data:         N x int16_t samples (little-endian)
    End frame:    0xFC 0x03

Requirements:
    pip install pyserial matplotlib numpy

Usage:
    python waveform_viewer.py COM3
"""

import sys
import struct
import threading
import time
import numpy as np
import serial

try:
    import matplotlib
    matplotlib.use("TkAgg")
    import matplotlib.pyplot as plt
    from matplotlib.animation import FuncAnimation
except ImportError:
    print("ERROR: matplotlib required. Install with: pip install matplotlib")
    sys.exit(1)


# ---- Configuration ----
SAMPLE_RATE = 48000
FRAME_SIZE = 64         # Samples per UART frame
BAUD_RATE = 115200
PLOT_HISTORY = 512      # Samples shown in waveform plot
FFT_SIZE = 512          # FFT window size

# ---- Global State ----
sample_buffer = np.zeros(PLOT_HISTORY, dtype=np.int16)
buffer_lock = threading.Lock()
new_data_flag = False


def serial_reader(port_name):
    """Background thread: read UART frames and parse audio samples."""
    global sample_buffer, new_data_flag

    try:
        ser = serial.Serial(port_name, BAUD_RATE, timeout=0.5)
        print(f"Waveform Viewer connected to {port_name}")
    except serial.SerialException as e:
        print(f"ERROR: Cannot open {port_name}: {e}")
        return

    state = "IDLE"
    frame_data = bytearray()

    while True:
        try:
            byte = ser.read(1)
            if not byte:
                continue
            b = byte[0]

            if state == "IDLE":
                if b == 0x03:
                    state = "START1"
            elif state == "START1":
                if b == 0xFC:
                    state = "DATA"
                    frame_data = bytearray()
                else:
                    state = "IDLE"
            elif state == "DATA":
                if b == 0xFC:
                    state = "END1"
                else:
                    frame_data.append(b)
            elif state == "END1":
                if b == 0x03:
                    # Complete frame received
                    if len(frame_data) >= 2:
                        n_samples = len(frame_data) // 2
                        samples = struct.unpack(
                            f"<{n_samples}h", frame_data[:n_samples * 2])

                        with buffer_lock:
                            # Shift buffer and append new samples
                            shift = min(n_samples, PLOT_HISTORY)
                            sample_buffer = np.roll(sample_buffer, -shift)
                            sample_buffer[-shift:] = samples[:shift]
                            new_data_flag = True

                    state = "IDLE"
                else:
                    # False end marker, treat as data
                    frame_data.append(0xFC)
                    frame_data.append(b)
                    state = "DATA"

        except serial.SerialException:
            print("Serial connection lost")
            break
        except Exception as e:
            print(f"Reader error: {e}")
            time.sleep(0.1)


def main():
    if len(sys.argv) < 2:
        print("Usage: python waveform_viewer.py <COM_PORT>")
        print("Example: python waveform_viewer.py COM3")
        sys.exit(1)

    port = sys.argv[1]

    # Start serial reader thread
    reader = threading.Thread(target=serial_reader, args=(port,), daemon=True)
    reader.start()

    # ---- Set up Matplotlib Plots ----
    fig, (ax_wave, ax_fft) = plt.subplots(2, 1, figsize=(10, 7))
    fig.suptitle("dsPIC33AK Audio Demo - Waveform Viewer", fontsize=14)

    # Waveform plot
    time_axis = np.arange(PLOT_HISTORY) / SAMPLE_RATE * 1000  # ms
    wave_line, = ax_wave.plot(time_axis, np.zeros(PLOT_HISTORY), color="cyan",
                               linewidth=0.8)
    ax_wave.set_xlim(0, time_axis[-1])
    ax_wave.set_ylim(-33000, 33000)
    ax_wave.set_xlabel("Time (ms)")
    ax_wave.set_ylabel("Amplitude")
    ax_wave.set_title("Waveform")
    ax_wave.grid(True, alpha=0.3)
    ax_wave.set_facecolor("#1a1a2e")

    # Spectrum plot
    freq_axis = np.fft.rfftfreq(FFT_SIZE, 1.0 / SAMPLE_RATE)
    fft_line, = ax_fft.plot(freq_axis, np.zeros(len(freq_axis)),
                             color="lime", linewidth=0.8)
    ax_fft.set_xlim(20, SAMPLE_RATE / 2)
    ax_fft.set_ylim(0, 100)
    ax_fft.set_xscale("log")
    ax_fft.set_xlabel("Frequency (Hz)")
    ax_fft.set_ylabel("Magnitude (dB)")
    ax_fft.set_title("Spectrum")
    ax_fft.grid(True, alpha=0.3)
    ax_fft.set_facecolor("#1a1a2e")

    fig.set_facecolor("#0e0e1a")
    for ax in [ax_wave, ax_fft]:
        ax.tick_params(colors="white")
        ax.xaxis.label.set_color("white")
        ax.yaxis.label.set_color("white")
        ax.title.set_color("white")

    plt.tight_layout()

    # ---- Animation Update Function ----
    def update(frame):
        global new_data_flag
        with buffer_lock:
            if not new_data_flag:
                return wave_line, fft_line
            data = sample_buffer.copy()
            new_data_flag = False

        # Update waveform
        wave_line.set_ydata(data)

        # Compute and update FFT
        windowed = data[-FFT_SIZE:] * np.hanning(FFT_SIZE)
        fft_mag = np.abs(np.fft.rfft(windowed))
        fft_db = 20 * np.log10(fft_mag + 1e-10)  # Avoid log(0)
        fft_db = np.clip(fft_db, 0, 100)
        fft_line.set_ydata(fft_db)

        return wave_line, fft_line

    ani = FuncAnimation(fig, update, interval=50, blit=True, cache_frame_data=False)

    print("Waveform Viewer running. Close the plot window to exit.")
    print("Enable streaming on the board: 'stream on'")
    plt.show()


if __name__ == "__main__":
    main()
