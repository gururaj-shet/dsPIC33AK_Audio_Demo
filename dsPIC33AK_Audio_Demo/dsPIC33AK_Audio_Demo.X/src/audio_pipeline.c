/**
 * Audio Processing Pipeline
 * 
 * @file      audio_pipeline.c
 * @brief     Complete audio DSP pipeline with DMA-driven I/O
 *
 * Signal flow:
 *   1. Audio input arrives via DMA (codec I2S or ADC for mic)
 *   2. DMA complete ISR signals processing needed
 *   3. Main loop (or high-priority task) processes the buffer:
 *      a. De-interleave stereo to mono (if needed)
 *      b. Noise reduction (simple noise gate)
 *      c. 5-band parametric equalizer
 *      d. Bass shelf boost/cut
 *      e. Treble shelf boost/cut
 *      f. Echo effect (delay line)
 *      g. Re-interleave to stereo
 *   4. Processed buffer is written to TX DMA buffer
 *   5. DMA sends to codec DAC
 *
 * Buffer management:
 *   - All processing is block-based (256 samples per block)
 *   - Double-buffered: DMA fills one, DSP processes the other
 *   - Processing must complete within one block period:
 *     256 / 48000 = 5.33 ms per block
 *   - At 200 MHz, that's ~1,066,667 instruction cycles per block
 */

#include "../include/audio_pipeline.h"
#include "../include/equalizer.h"
#include "../include/codec_driver.h"
#include "../mcc_generated_files/dma/dma.h"
#include "../mcc_generated_files/spi/spi1.h"
#include "../mcc_generated_files/adc/adc5.h"
#include "../mcc_generated_files/uart/uart1.h"
#include "../mcc_generated_files/system/pins.h"
#include <string.h>
#include <math.h>

/* ---- Pipeline State ---- */
static audio_config_t pipelineConfig;
static volatile bool processingPending = false;

/* Mono processing buffer (extracted from stereo) */
static int16_t monoBuffer[AUDIO_BLOCK_SIZE];

/* Echo delay buffer (circular) */
#define ECHO_MAX_DELAY_SAMPLES  ((uint16_t)(AUDIO_SAMPLE_RATE_HZ / 2))  /* 500ms max */
static int16_t echoBuffer[24000];   /* 500ms at 48kHz */
static uint16_t echoWriteIdx = 0;
static uint16_t echoReadIdx = 0;

/* Test tone generator state */
static uint32_t tonePhaseAccum = 0;
static uint32_t tonePhaseIncr = 0;

/* Sine lookup table (256 entries, Q15) */
static const int16_t sineLUT[256] = {
        0,   804,  1608,  2410,  3212,  4011,  4808,  5602,
     6393,  7179,  7962,  8739,  9512, 10278, 11039, 11793,
    12539, 13279, 14010, 14732, 15446, 16151, 16846, 17530,
    18204, 18868, 19519, 20159, 20787, 21403, 22005, 22594,
    23170, 23731, 24279, 24811, 25329, 25832, 26319, 26790,
    27245, 27683, 28105, 28510, 28898, 29268, 29621, 29956,
    30273, 30571, 30852, 31113, 31356, 31580, 31785, 31971,
    32137, 32285, 32412, 32521, 32609, 32678, 32728, 32757,
    32767, 32757, 32728, 32678, 32609, 32521, 32412, 32285,
    32137, 31971, 31785, 31580, 31356, 31113, 30852, 30571,
    30273, 29956, 29621, 29268, 28898, 28510, 28105, 27683,
    27245, 26790, 26319, 25832, 25329, 24811, 24279, 23731,
    23170, 22594, 22005, 21403, 20787, 20159, 19519, 18868,
    18204, 17530, 16846, 16151, 15446, 14732, 14010, 13279,
    12539, 11793, 11039, 10278,  9512,  8739,  7962,  7179,
     6393,  5602,  4808,  4011,  3212,  2410,  1608,   804,
        0,  -804, -1608, -2410, -3212, -4011, -4808, -5602,
    -6393, -7179, -7962, -8739, -9512,-10278,-11039,-11793,
   -12539,-13279,-14010,-14732,-15446,-16151,-16846,-17530,
   -18204,-18868,-19519,-20159,-20787,-21403,-22005,-22594,
   -23170,-23731,-24279,-24811,-25329,-25832,-26319,-26790,
   -27245,-27683,-28105,-28510,-28898,-29268,-29621,-29956,
   -30273,-30571,-30852,-31113,-31356,-31580,-31785,-31971,
   -32137,-32285,-32412,-32521,-32609,-32678,-32728,-32757,
   -32767,-32757,-32728,-32678,-32609,-32521,-32412,-32285,
   -32137,-31971,-31785,-31580,-31356,-31113,-30852,-30571,
   -30273,-29956,-29621,-29268,-28898,-28510,-28105,-27683,
   -27245,-26790,-26319,-25832,-25329,-24811,-24279,-23731,
   -23170,-22594,-22005,-21403,-20787,-20159,-19519,-18868,
   -18204,-17530,-16846,-16151,-15446,-14732,-14010,-13279,
   -12539,-11793,-11039,-10278, -9512, -8739, -7962, -7179,
    -6393, -5602, -4808, -4011, -3212, -2410, -1608,  -804,
};

/* ---- Noise Reduction (Simple Noise Gate) ---- */

#define NOISE_GATE_THRESHOLD    200     /* Below this amplitude = silence */
#define NOISE_GATE_ATTACK_MS    2       /* Attack time in ms */
#define NOISE_GATE_RELEASE_MS   50      /* Release time in ms */

static int16_t noiseGateLevel = 0;
static uint16_t noiseGateHoldCount = 0;

/**
 * @brief Simple noise gate - mutes signal below threshold
 */
static void NoiseGate_Process(int16_t *samples, uint16_t count)
{
    uint16_t i;
    for(i = 0; i < count; i++)
    {
        int16_t absVal = (samples[i] >= 0) ? samples[i] : -samples[i];

        if(absVal > NOISE_GATE_THRESHOLD)
        {
            /* Signal above threshold - gate is open */
            noiseGateHoldCount = (NOISE_GATE_RELEASE_MS * AUDIO_SAMPLE_RATE_HZ) / 1000;
        }
        else if(noiseGateHoldCount > 0)
        {
            /* In release phase */
            noiseGateHoldCount--;
        }
        else
        {
            /* Gate closed - mute */
            samples[i] = 0;
        }
    }
}

/* ---- Echo Effect ---- */

/**
 * @brief Apply delay-based echo effect
 * @param samples Audio buffer (in-place)
 * @param count Number of samples
 * @param delayMs Echo delay in milliseconds
 * @param decay Decay factor 0-10 (0=no echo, 10=long tail)
 */
static void Echo_Process(int16_t *samples, uint16_t count,
                         uint16_t delayMs, uint8_t decay)
{
    uint16_t i;
    uint16_t delaySamples = (uint16_t)((uint32_t)delayMs * AUDIO_SAMPLE_RATE_HZ / 1000U);
    int16_t decayFactor;

    if(delaySamples > ECHO_MAX_DELAY_SAMPLES)
        delaySamples = ECHO_MAX_DELAY_SAMPLES;

    /* Map decay 0-10 to Q15 multiplier */
    decayFactor = (int16_t)((int32_t)decay * 3276);  /* ~0.1 per step */

    for(i = 0; i < count; i++)
    {
        /* Read delayed sample */
        uint16_t readIdx = (echoWriteIdx >= delaySamples) ?
                           (echoWriteIdx - delaySamples) :
                           (ECHO_MAX_DELAY_SAMPLES - delaySamples + echoWriteIdx);
        int16_t delayed = echoBuffer[readIdx];

        /* Mix: output = input + delayed * decay */
        int32_t mixed = (int32_t)samples[i] +
                        (((int32_t)delayed * (int32_t)decayFactor) >> 15);

        /* Saturate */
        if(mixed > 32767) mixed = 32767;
        if(mixed < -32768) mixed = -32768;

        /* Write current sample + feedback to delay buffer */
        echoBuffer[echoWriteIdx] = (int16_t)mixed;
        echoWriteIdx++;
        if(echoWriteIdx >= ECHO_MAX_DELAY_SAMPLES)
            echoWriteIdx = 0;

        samples[i] = (int16_t)mixed;
    }
}

/* ---- Test Tone Generator ---- */

/**
 * @brief Generate a sine wave test tone
 */
static void TestTone_Generate(int16_t *buffer, uint16_t count)
{
    uint16_t i;
    for(i = 0; i < count; i++)
    {
        /* Phase accumulator (24.8 fixed point) */
        uint8_t index = (uint8_t)(tonePhaseAccum >> 24);
        buffer[i] = sineLUT[index];
        tonePhaseAccum += tonePhaseIncr;
    }
}

/**
 * @brief Set test tone frequency
 */
static void TestTone_SetFrequency(uint16_t freqHz)
{
    /* Phase increment = freq * 2^32 / Fs */
    tonePhaseIncr = ((uint32_t)freqHz * (uint32_t)0x100000) /
                    (AUDIO_SAMPLE_RATE_HZ / 256);
}

/* ---- DMA Callbacks ---- */

/**
 * @brief Called from DMA ISR when codec RX buffer is complete
 */
static void AudioRxComplete(void)
{
    processingPending = true;
    LED_STATUS_Toggle();    /* Visual indicator of audio activity */
}

/**
 * @brief Called from DMA ISR when ADC mic buffer is complete
 */
static void MicRxComplete(void)
{
    if(pipelineConfig.inputSource == AUDIO_INPUT_MIC2)
    {
        processingPending = true;
    }
}

/* ---- Public API ---- */

void AudioPipeline_Initialize(void)
{
    /* Default configuration */
    memset(&pipelineConfig, 0, sizeof(pipelineConfig));
    pipelineConfig.inputSource = AUDIO_INPUT_CODEC;
    pipelineConfig.volume = 200;
    pipelineConfig.bassLevel = 5;       /* Flat */
    pipelineConfig.trebleLevel = 5;     /* Flat */
    pipelineConfig.effects.equalizerEnabled = true;
    pipelineConfig.effects.noiseReduction = false;
    pipelineConfig.effects.bassBoost = false;
    pipelineConfig.effects.trebleBoost = false;
    pipelineConfig.effects.echoEnabled = false;
    pipelineConfig.effects.reverbEnabled = false;
    pipelineConfig.echoDelay = 200;     /* 200 ms */
    pipelineConfig.echoDecay = 5;
    pipelineConfig.testToneFreq = 1000; /* 1 kHz */

    /* Set all EQ bands to flat */
    uint8_t i;
    for(i = 0; i < 5; i++)
        pipelineConfig.eqBand[i] = 5;

    /* Initialize DSP modules */
    EQ_Initialize();
    BassTreble_Initialize();

    /* Clear echo buffer */
    memset(echoBuffer, 0, sizeof(echoBuffer));
    echoWriteIdx = 0;

    /* Register DMA callbacks */
    DMA_Channel1TransferDoneCallbackRegister(AudioRxComplete);
    DMA_Channel2TransferDoneCallbackRegister(MicRxComplete);

    /* Initialize codec */
    CODEC_Initialize();
    CODEC_SetVolume(pipelineConfig.volume);
    CODEC_SetInputSource(CODEC_INPUT_LINE);

    /* Set test tone frequency */
    TestTone_SetFrequency(pipelineConfig.testToneFreq);
}

void AudioPipeline_Start(void)
{
    /* Enable DMA channels */
    DMA_Channel0Enable();   /* SPI1 TX */
    DMA_Channel1Enable();   /* SPI1 RX */

    if(pipelineConfig.inputSource == AUDIO_INPUT_MIC2)
    {
        DMA_Channel2Enable();   /* ADC */
        ADC5_Enable();
    }

    /* Start SPI1 (I2S) */
    SPI1_Enable();
}

void AudioPipeline_Stop(void)
{
    SPI1_Disable();
    DMA_DisableAll();
    ADC5_Disable();
}

void AudioPipeline_ProcessBlock(int16_t *input, int16_t *output,
                                uint16_t blockSize)
{
    uint16_t i;
    bool clipDetected = false;

    /* Step 1: De-interleave stereo to mono (average L+R) or use mono input */
    if(pipelineConfig.inputSource == AUDIO_INPUT_TEST_TONE)
    {
        /* Generate test tone */
        TestTone_Generate(monoBuffer, blockSize);
    }
    else if(pipelineConfig.inputSource == AUDIO_INPUT_MIC2)
    {
        /* MIC 2 Click is mono - copy directly */
        int16_t *adcBuf = DMA_AdcBufferGet();
        memcpy(monoBuffer, adcBuf, blockSize * sizeof(int16_t));
    }
    else
    {
        /* Codec input: de-interleave stereo (take left channel) */
        for(i = 0; i < blockSize; i++)
        {
            monoBuffer[i] = input[i * 2];  /* Left channel */
        }
    }

    /* Step 2: Noise reduction */
    if(pipelineConfig.effects.noiseReduction)
    {
        NoiseGate_Process(monoBuffer, blockSize);
    }

    /* Step 3: 5-band equalizer */
    if(pipelineConfig.effects.equalizerEnabled)
    {
        EQ_Process(monoBuffer, blockSize);
    }

    /* Step 4: Bass boost/cut */
    if(pipelineConfig.effects.bassBoost)
    {
        Bass_Process(monoBuffer, blockSize, pipelineConfig.bassLevel);
    }

    /* Step 5: Treble boost/cut */
    if(pipelineConfig.effects.trebleBoost)
    {
        Treble_Process(monoBuffer, blockSize, pipelineConfig.trebleLevel);
    }

    /* Step 6: Echo effect */
    if(pipelineConfig.effects.echoEnabled)
    {
        Echo_Process(monoBuffer, blockSize,
                     pipelineConfig.echoDelay,
                     pipelineConfig.echoDecay);
    }

    /* Step 7: Re-interleave to stereo output and apply volume */
    for(i = 0; i < blockSize; i++)
    {
        int32_t scaled = ((int32_t)monoBuffer[i] * (int32_t)pipelineConfig.volume) >> 8;

        /* Clip detection */
        if(scaled > 32767 || scaled < -32768)
            clipDetected = true;

        /* Saturate */
        if(scaled > 32767) scaled = 32767;
        if(scaled < -32768) scaled = -32768;

        int16_t sample = (int16_t)scaled;
        output[i * 2]     = sample;     /* Left */
        output[i * 2 + 1] = sample;     /* Right (mono -> both channels) */
    }

    /* Update clip LED */
    if(clipDetected)
        LED_CLIP_SetHigh();
    else
        LED_CLIP_SetLow();
}

audio_config_t* AudioPipeline_GetConfig(void)
{
    return &pipelineConfig;
}

void AudioPipeline_SetInputSource(audio_input_source_t source)
{
    pipelineConfig.inputSource = source;

    if(source == AUDIO_INPUT_CODEC)
    {
        CODEC_SetInputSource(CODEC_INPUT_LINE);
    }
    else if(source == AUDIO_INPUT_MIC2)
    {
        /* Enable ADC DMA for microphone */
        DMA_Channel2Enable();
        ADC5_Enable();
    }
}

void AudioPipeline_SetEffects(audio_effects_t *effects)
{
    memcpy(&pipelineConfig.effects, effects, sizeof(audio_effects_t));
}

void AudioPipeline_StreamSamples(int16_t *buffer, uint16_t count)
{
    uint16_t i;

    /* MPLAB Data Visualizer protocol:
     * Start frame marker: 0x03 0xFC
     * Then send raw int16 samples (little-endian)
     * End frame marker: 0xFC 0x03
     */
    UART1_Write(0x03);
    UART1_Write(0xFC);

    for(i = 0; i < count; i++)
    {
        uint16_t sample = (uint16_t)buffer[i];
        UART1_Write((uint8_t)(sample & 0xFF));
        UART1_Write((uint8_t)((sample >> 8) & 0xFF));
    }

    UART1_Write(0xFC);
    UART1_Write(0x03);
}
