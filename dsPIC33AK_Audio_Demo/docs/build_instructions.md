# Build Instructions

## Prerequisites

### Software
1. **MPLAB X IDE** v6.25 or later
   - Download: https://www.microchip.com/mplab/mplab-x-ide
2. **XC-DSC Compiler** v3.21 or later
   - Download: https://www.microchip.com/xc-dsc
3. **dsPIC33AK-MC_DFP** v1.0.4 or later (Device Family Pack)
   - Installed via MPLAB X Pack Manager
4. **Python 3.8+** (for GUI tools)
   - `pip install pyserial matplotlib numpy`

### Hardware
- See `hardware_connection.md` for complete hardware setup

## Step 1: Open Project in MPLAB X

1. Launch **MPLAB X IDE**
2. Go to **File > Open Project**
3. Navigate to `dsPIC33AK_Audio_Demo/dsPIC33AK_Audio_Demo.X/`
4. Select the project folder (it contains `Makefile` and `nbproject/`)
5. Click **Open Project**

## Step 2: Verify Compiler Settings

1. Right-click the project in the Projects panel
2. Select **Properties**
3. Under **Conf: [default]**:
   - **Device**: `dsPIC33AK512MPS512`
   - **Hardware Tool**: Select your debugger (e.g., PKOB4 on Curiosity board)
   - **Compiler Toolchain**: `XC-DSC (v3.21+)`
4. Under **XC-DSC (Global Options) > xc-dsc-gcc**:
   - **Optimization Level**: `-O1` (recommended for debugging) or `-O2` for production
   - Ensure `Include directories` contains:
     - `./include`
     - `./mcc_generated_files`
5. Under **xc-dsc-ld** (Linker):
   - **Heap Size**: `4096` (needed for sprintf)

## Step 3: Build the Project

1. Click the **Build** button (hammer icon) or press **F11**
2. The Output window should show:
   ```
   BUILD SUCCESSFUL (total time: Xs)
   ```
3. If you see errors about missing device headers, verify the DFP is installed:
   - **Tools > Packs** and search for `dsPIC33AK`

## Step 4: Program the Board

1. Connect the Curiosity Platform Board via USB-C
2. Ensure the dsPIC33AK512MPS512 DIM is properly seated
3. Click **Run > Run Main Project** (or press **F6**)
   - This will build, program, and start the application
4. Alternatively, click the **Make and Program Device** button

## Step 5: Connect to UART Console

1. Open a serial terminal program (PuTTY, Tera Term, or MPLAB X Terminal)
2. Settings: **115200 baud, 8-N-1, no flow control**
3. Select the COM port associated with the Curiosity board
   - Note: The board may appear as 2 COM ports; try each one
4. You should see the startup banner:
   ```
   ========================================
     dsPIC33AK512MPS512 Audio DSP Demo
     Ported from dsPIC33F/E to dsPIC33A
   ========================================
   ```
5. Type `help` to see available commands

## Step 6: Run the Python GUI (Optional)

### Control GUI
```bash
cd python_gui
python audio_control_gui.py COM3
```
Replace `COM3` with your actual COM port.

### Waveform Viewer
```bash
cd python_gui
python waveform_viewer.py COM3
```
Note: Enable streaming first via CLI (`stream on`) or the GUI checkbox.

## Step 7: Test Audio

### Test Mode 1: Loopback
1. Connect an audio source to the codec line-in
2. Connect a speaker to the codec headphone out
3. Type `mode 1` in the CLI
4. Speak into the MIC 2 Click or play audio into line-in
5. You should hear the processed audio from the speaker

### Test Mode 2: Test Tone
1. Type `mode 2` in the CLI
2. A 1 kHz sine wave plays through the speaker
3. Adjust frequency: `freq 440` (for A4 note)
4. Apply effects: `bass 8`, `echo on`, etc.

### Test Mode 3: Visualizer
1. Type `mode 3` in the CLI
2. Run the waveform viewer: `python waveform_viewer.py COM3`
3. Speak into the MIC 2 Click
4. See real-time waveform and spectrum on your PC

## Troubleshooting

| Problem | Solution |
|---------|----------|
| No audio output | Check codec connections, verify MCLK on RD10 |
| UART not responding | Try the other COM port, verify 115200 baud |
| Build fails with DFP errors | Install dsPIC33AK-MC_DFP via Pack Manager |
| Codec not responding on I2C | Check pull-ups, verify RE3/RE4 are high |
| MIC 2 Click too quiet | Increase gain: `gain 200` |
| Audio clicks/pops | Check DMA buffer sizes, reduce processing load |
| Python GUI won't connect | Install pyserial: `pip install pyserial` |

## MCC Regeneration (Advanced)

If you need to modify the MCC configuration:

1. Open the `.mc3` file in MPLAB X MCC Melody
2. Modify peripheral settings as needed
3. Click **Generate** to update `mcc_generated_files/`
4. **Warning**: Manual edits to MCC files will be overwritten

The current project uses manually-tuned MCC-style files optimized for
audio. Only regenerate if you need to change fundamental peripheral settings.

## Compiler Optimization Notes

For best DSP performance in production:
- Use `-O2` optimization level
- Enable `-ffast-math` for floating-point operations
- The Q15 fixed-point code does not require `-ffast-math` but benefits from `-O2`

For debugging:
- Use `-O0` or `-O1`
- Enable `-g` for debug symbols (default in Debug configuration)
