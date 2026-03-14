/**
 * dsPIC33AK Audio Demo - Main Application
 * 
 * @file      main.c
 * @brief     Complete audio DSP demo for dsPIC33AK512MPS512
 * @version   1.0.0
 * @date      2026
 *
 * Hardware:
 *   - dsPIC33AK Curiosity Platform Board (EV74H48A)
 *   - dsPIC33AK512MPS512 GP DIM (EV80L65A)
 *   - AC328904 Audio Codec Board (AK4642 codec)
 *   - MIC 2 Click microphone board (optional)
 *   - External speaker via codec headphone/line out
 *
 * Features:
 *   - DMA-based audio streaming with double-buffering
 *   - 5-band parametric equalizer
 *   - Bass and treble boost/cut
 *   - Noise reduction (noise gate)
 *   - Echo/delay effect
 *   - UART CLI for real-time control
 *   - Audio sample streaming for PC visualization
 *   - Test tone generator
 *
 * Demo Modes:
 *   MODE 1: Loopback (Mic/Codec -> DSP -> Speaker)
 *   MODE 2: Test Tone Generator (internal sine -> Speaker)
 *   MODE 3: Spectrum Visualizer (Mic -> PC GUI via UART)
 *
 * UART CLI Commands (115200 8-N-1):
 *   bass 0-10      Set bass level
 *   treble 0-10    Set treble level
 *   eq1-5 0-10     Set equalizer band
 *   echo on/off    Enable/disable echo
 *   noise on/off   Enable/disable noise gate
 *   vol 0-255      Set volume
 *   mode 1-3       Select demo mode
 *   input codec/mic/tone  Select input source
 *   stream on/off  Enable/disable sample streaming
 *   status         Print current settings
 *   help           Print command list
 *
 * Ported from dsPIC33F/E audio library to dsPIC33A architecture.
 * All peripheral registers updated for dsPIC33AK512MPS512.
 */

#include "mcc_generated_files/system/system.h"
#include "mcc_generated_files/timer/tmr1.h"
#include "mcc_generated_files/uart/uart1.h"
#include "mcc_generated_files/dma/dma.h"
#include "mcc_generated_files/system/pins.h"
#include "include/audio_pipeline.h"
#include "include/equalizer.h"
#include "include/codec_driver.h"
#include "include/mic_driver.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* ---- Demo Mode Definitions ---- */
typedef enum {
    DEMO_MODE_LOOPBACK = 1,     /* Mic/Codec -> DSP -> Speaker */
    DEMO_MODE_TEST_TONE = 2,    /* Internal test tone -> Speaker */
    DEMO_MODE_VISUALIZER = 3    /* Mic -> UART stream to PC */
} demo_mode_t;

/* ---- Application State ---- */
static demo_mode_t currentMode = DEMO_MODE_LOOPBACK;
static bool streamingEnabled = false;
static volatile uint32_t systemTickMs = 0;

/* ---- CLI Parser State ---- */
#define CLI_BUFFER_SIZE     64
static char cliBuffer[CLI_BUFFER_SIZE];
static uint8_t cliIndex = 0;

/* ---- System Tick Handler ---- */

/**
 * @brief 1 ms timer interrupt callback
 */
static void SystemTick_Handler(void)
{
    systemTickMs++;
}

/* ---- UART Helpers ---- */

static void UART_PrintString(const char *str)
{
    while(*str)
    {
        while(!UART1_IsTxReady()) {}
        UART1_Write((uint8_t)*str++);
    }
}

static void UART_PrintLine(const char *str)
{
    UART_PrintString(str);
    UART_PrintString("\r\n");
}

static void UART_PrintInt(int32_t value)
{
    char buf[12];
    sprintf(buf, "%ld", value);
    UART_PrintString(buf);
}

/* ---- CLI Command Processing ---- */

static void PrintBanner(void)
{
    UART_PrintLine("");
    UART_PrintLine("========================================");
    UART_PrintLine("  dsPIC33AK512MPS512 Audio DSP Demo");
    UART_PrintLine("  Ported from dsPIC33F/E to dsPIC33A");
    UART_PrintLine("========================================");
    UART_PrintLine("");
}

static void PrintHelp(void)
{
    UART_PrintLine("Available commands:");
    UART_PrintLine("  bass <0-10>        Set bass level (5=flat)");
    UART_PrintLine("  treble <0-10>      Set treble level (5=flat)");
    UART_PrintLine("  eq<1-5> <0-10>     Set EQ band (5=flat)");
    UART_PrintLine("  echo <on/off>      Toggle echo effect");
    UART_PrintLine("  noise <on/off>     Toggle noise gate");
    UART_PrintLine("  vol <0-255>        Set output volume");
    UART_PrintLine("  mode <1-3>         1=Loopback 2=Tone 3=Visualizer");
    UART_PrintLine("  input <codec/mic/tone>  Select input source");
    UART_PrintLine("  stream <on/off>    Toggle sample streaming");
    UART_PrintLine("  freq <Hz>          Set test tone frequency");
    UART_PrintLine("  gain <0-255>       Set mic gain (MIC 2 Click)");
    UART_PrintLine("  status             Show current settings");
    UART_PrintLine("  help               Show this help");
    UART_PrintLine("");
}

static void PrintStatus(void)
{
    audio_config_t *cfg = AudioPipeline_GetConfig();

    UART_PrintLine("--- Current Settings ---");
    UART_PrintString("  Mode:      "); UART_PrintInt(currentMode); UART_PrintLine("");
    UART_PrintString("  Input:     ");
    switch(cfg->inputSource) {
        case AUDIO_INPUT_CODEC:     UART_PrintLine("Codec"); break;
        case AUDIO_INPUT_MIC2:      UART_PrintLine("MIC 2 Click"); break;
        case AUDIO_INPUT_TEST_TONE: UART_PrintLine("Test Tone"); break;
    }
    UART_PrintString("  Volume:    "); UART_PrintInt(cfg->volume); UART_PrintLine("");
    UART_PrintString("  Bass:      "); UART_PrintInt(cfg->bassLevel); UART_PrintLine("");
    UART_PrintString("  Treble:    "); UART_PrintInt(cfg->trebleLevel); UART_PrintLine("");
    UART_PrintString("  EQ Bands:  ");
    for(int i = 0; i < 5; i++) {
        UART_PrintInt(cfg->eqBand[i]);
        UART_PrintString(" ");
    }
    UART_PrintLine("");
    UART_PrintString("  Noise:     "); UART_PrintLine(cfg->effects.noiseReduction ? "ON" : "OFF");
    UART_PrintString("  Echo:      "); UART_PrintLine(cfg->effects.echoEnabled ? "ON" : "OFF");
    UART_PrintString("  Stream:    "); UART_PrintLine(streamingEnabled ? "ON" : "OFF");
    UART_PrintLine("------------------------");
}

static void ProcessCommand(char *cmd)
{
    audio_config_t *cfg = AudioPipeline_GetConfig();
    char *arg;
    int value;

    /* Trim leading whitespace */
    while(*cmd == ' ') cmd++;

    /* Skip empty commands */
    if(strlen(cmd) == 0) return;

    /* Parse command */
    if(strncmp(cmd, "help", 4) == 0)
    {
        PrintHelp();
    }
    else if(strncmp(cmd, "status", 6) == 0)
    {
        PrintStatus();
    }
    else if(strncmp(cmd, "bass ", 5) == 0)
    {
        value = atoi(cmd + 5);
        if(value >= 0 && value <= 10) {
            cfg->bassLevel = (uint8_t)value;
            cfg->effects.bassBoost = true;
            UART_PrintString("Bass: "); UART_PrintInt(value); UART_PrintLine("");
        }
    }
    else if(strncmp(cmd, "treble ", 7) == 0)
    {
        value = atoi(cmd + 7);
        if(value >= 0 && value <= 10) {
            cfg->trebleLevel = (uint8_t)value;
            cfg->effects.trebleBoost = true;
            UART_PrintString("Treble: "); UART_PrintInt(value); UART_PrintLine("");
        }
    }
    else if(strncmp(cmd, "eq", 2) == 0 && cmd[2] >= '1' && cmd[2] <= '5')
    {
        int band = cmd[2] - '1';
        value = atoi(cmd + 4);
        if(value >= 0 && value <= 10) {
            cfg->eqBand[band] = (uint8_t)value;
            EQ_SetBandGain((uint8_t)band, (uint8_t)value);
            UART_PrintString("EQ Band "); UART_PrintInt(band + 1);
            UART_PrintString(": "); UART_PrintInt(value); UART_PrintLine("");
        }
    }
    else if(strncmp(cmd, "echo ", 5) == 0)
    {
        arg = cmd + 5;
        if(strncmp(arg, "on", 2) == 0) {
            cfg->effects.echoEnabled = true;
            UART_PrintLine("Echo: ON");
        } else {
            cfg->effects.echoEnabled = false;
            UART_PrintLine("Echo: OFF");
        }
    }
    else if(strncmp(cmd, "noise ", 6) == 0)
    {
        arg = cmd + 6;
        if(strncmp(arg, "on", 2) == 0) {
            cfg->effects.noiseReduction = true;
            UART_PrintLine("Noise gate: ON");
        } else {
            cfg->effects.noiseReduction = false;
            UART_PrintLine("Noise gate: OFF");
        }
    }
    else if(strncmp(cmd, "vol ", 4) == 0)
    {
        value = atoi(cmd + 4);
        if(value >= 0 && value <= 255) {
            cfg->volume = (uint8_t)value;
            CODEC_SetVolume((uint8_t)value);
            UART_PrintString("Volume: "); UART_PrintInt(value); UART_PrintLine("");
        }
    }
    else if(strncmp(cmd, "mode ", 5) == 0)
    {
        value = atoi(cmd + 5);
        if(value >= 1 && value <= 3) {
            currentMode = (demo_mode_t)value;
            switch(currentMode) {
                case DEMO_MODE_LOOPBACK:
                    AudioPipeline_SetInputSource(AUDIO_INPUT_CODEC);
                    UART_PrintLine("Mode: Loopback");
                    break;
                case DEMO_MODE_TEST_TONE:
                    AudioPipeline_SetInputSource(AUDIO_INPUT_TEST_TONE);
                    UART_PrintLine("Mode: Test Tone");
                    break;
                case DEMO_MODE_VISUALIZER:
                    AudioPipeline_SetInputSource(AUDIO_INPUT_MIC2);
                    streamingEnabled = true;
                    UART_PrintLine("Mode: Visualizer (streaming ON)");
                    break;
            }
        }
    }
    else if(strncmp(cmd, "input ", 6) == 0)
    {
        arg = cmd + 6;
        if(strncmp(arg, "codec", 5) == 0) {
            AudioPipeline_SetInputSource(AUDIO_INPUT_CODEC);
            UART_PrintLine("Input: Codec");
        } else if(strncmp(arg, "mic", 3) == 0) {
            AudioPipeline_SetInputSource(AUDIO_INPUT_MIC2);
            UART_PrintLine("Input: MIC 2 Click");
        } else if(strncmp(arg, "tone", 4) == 0) {
            AudioPipeline_SetInputSource(AUDIO_INPUT_TEST_TONE);
            UART_PrintLine("Input: Test Tone");
        }
    }
    else if(strncmp(cmd, "stream ", 7) == 0)
    {
        arg = cmd + 7;
        streamingEnabled = (strncmp(arg, "on", 2) == 0);
        UART_PrintLine(streamingEnabled ? "Streaming: ON" : "Streaming: OFF");
    }
    else if(strncmp(cmd, "freq ", 5) == 0)
    {
        value = atoi(cmd + 5);
        if(value >= 20 && value <= 20000) {
            cfg->testToneFreq = (uint16_t)value;
            UART_PrintString("Test tone: "); UART_PrintInt(value);
            UART_PrintLine(" Hz");
        }
    }
    else if(strncmp(cmd, "gain ", 5) == 0)
    {
        value = atoi(cmd + 5);
        if(value >= 0 && value <= 255) {
            MIC2_SetGain((uint8_t)value);
            UART_PrintString("Mic gain: "); UART_PrintInt(value); UART_PrintLine("");
        }
    }
    else
    {
        UART_PrintString("Unknown command: ");
        UART_PrintLine(cmd);
        UART_PrintLine("Type 'help' for available commands");
    }
}

/**
 * @brief Poll UART for CLI input characters
 */
static void CLI_Poll(void)
{
    if(!UART1_IsRxReady()) return;

    char c = (char)UART1_Read();

    /* Echo character */
    while(!UART1_IsTxReady()) {}
    UART1_Write((uint8_t)c);

    if(c == '\r' || c == '\n')
    {
        UART_PrintLine("");   /* New line */
        cliBuffer[cliIndex] = '\0';
        if(cliIndex > 0)
        {
            ProcessCommand(cliBuffer);
        }
        UART_PrintString("> ");
        cliIndex = 0;
    }
    else if(c == '\b' || c == 0x7F)
    {
        /* Backspace */
        if(cliIndex > 0) cliIndex--;
    }
    else if(cliIndex < CLI_BUFFER_SIZE - 1)
    {
        cliBuffer[cliIndex++] = c;
    }
}

/* ---- Button Handling ---- */

static uint32_t lastButtonTime = 0;
#define BUTTON_DEBOUNCE_MS  200

static void CheckButtons(void)
{
    if(systemTickMs - lastButtonTime < BUTTON_DEBOUNCE_MS)
        return;

    /* S1 button: Cycle through demo modes */
    if(!BUTTON_S1_GetValue())   /* Active low */
    {
        lastButtonTime = systemTickMs;
        currentMode++;
        if(currentMode > DEMO_MODE_VISUALIZER)
            currentMode = DEMO_MODE_LOOPBACK;

        switch(currentMode) {
            case DEMO_MODE_LOOPBACK:
                AudioPipeline_SetInputSource(AUDIO_INPUT_CODEC);
                UART_PrintLine("Mode: Loopback");
                break;
            case DEMO_MODE_TEST_TONE:
                AudioPipeline_SetInputSource(AUDIO_INPUT_TEST_TONE);
                UART_PrintLine("Mode: Test Tone");
                break;
            case DEMO_MODE_VISUALIZER:
                AudioPipeline_SetInputSource(AUDIO_INPUT_MIC2);
                streamingEnabled = true;
                UART_PrintLine("Mode: Visualizer");
                break;
        }
    }

    /* S2 button: Cycle through effects presets */
    if(!BUTTON_S2_GetValue())
    {
        lastButtonTime = systemTickMs;
        audio_config_t *cfg = AudioPipeline_GetConfig();

        /* Toggle through: Flat -> Bass Boost -> Echo -> All On */
        static uint8_t preset = 0;
        preset = (preset + 1) % 4;

        switch(preset) {
            case 0: /* Flat */
                cfg->effects.bassBoost = false;
                cfg->effects.trebleBoost = false;
                cfg->effects.echoEnabled = false;
                cfg->effects.noiseReduction = false;
                UART_PrintLine("Preset: Flat");
                break;
            case 1: /* Bass Boost */
                cfg->effects.bassBoost = true;
                cfg->bassLevel = 8;
                cfg->effects.trebleBoost = false;
                cfg->effects.echoEnabled = false;
                UART_PrintLine("Preset: Bass Boost");
                break;
            case 2: /* Echo */
                cfg->effects.echoEnabled = true;
                cfg->echoDelay = 200;
                cfg->echoDecay = 5;
                UART_PrintLine("Preset: Echo");
                break;
            case 3: /* All effects */
                cfg->effects.bassBoost = true;
                cfg->effects.trebleBoost = true;
                cfg->effects.echoEnabled = true;
                cfg->effects.noiseReduction = true;
                cfg->effects.equalizerEnabled = true;
                UART_PrintLine("Preset: All Effects");
                break;
        }
    }
}

/* ---- Main Application ---- */

int main(void)
{
    /* Initialize all system peripherals */
    SYSTEM_Initialize();

    /* Register 1 ms tick handler */
    TMR1_TimeoutCallbackRegister(&SystemTick_Handler);
    TMR1_Start();

    /* Configure LED outputs */
    LED_STATUS_SetDigitalOutput();
    LED_CLIP_SetDigitalOutput();
    LED_STATUS_SetLow();
    LED_CLIP_SetLow();

    /* Configure button inputs */
    BUTTON_S1_SetDigitalInput();
    BUTTON_S2_SetDigitalInput();

    /* Initialize MIC 2 Click */
    MIC2_Initialize();

    /* Initialize audio pipeline (includes codec init) */
    AudioPipeline_Initialize();

    /* Print startup banner */
    PrintBanner();
    PrintHelp();
    PrintStatus();
    UART_PrintString("> ");

    /* Start audio streaming */
    AudioPipeline_Start();

    /* ---- Main Loop ---- */
    while(1)
    {
        /* Process UART CLI commands */
        CLI_Poll();

        /* Check hardware buttons */
        CheckButtons();

        /* Audio processing: check if a new buffer is ready */
        /* The DMA ISR sets a flag; main loop processes the buffer */
        {
            int16_t *rxBuf = DMA_AudioRxBufferGet();
            int16_t *txBuf = DMA_AudioTxBufferGet();

            /* Process one block through the DSP pipeline */
            AudioPipeline_ProcessBlock(rxBuf, txBuf, AUDIO_BLOCK_SIZE);

            /* Stream samples to PC if enabled */
            if(streamingEnabled)
            {
                /* Stream every 4th block to avoid UART bottleneck */
                static uint8_t streamDiv = 0;
                if(++streamDiv >= 4)
                {
                    streamDiv = 0;
                    /* Stream 64 samples from mono buffer */
                    AudioPipeline_StreamSamples(rxBuf, 64);
                }
            }
        }
    }

    return 0;
}
