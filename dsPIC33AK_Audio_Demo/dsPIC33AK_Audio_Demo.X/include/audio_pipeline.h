/**
 * Audio Pipeline Header
 * 
 * @file      audio_pipeline.h
 * @brief     Audio processing pipeline control API
 *
 * Pipeline architecture:
 *   Input (Codec ADC or MIC 2 Click) 
 *     -> DMA Buffer
 *       -> Noise Reduction
 *         -> 5-Band Equalizer
 *           -> Bass Boost
 *             -> Treble Boost
 *               -> Echo/Reverb (optional)
 *                 -> DMA Output Buffer
 *                   -> Codec DAC / Speaker
 */

#ifndef AUDIO_PIPELINE_H
#define AUDIO_PIPELINE_H

#include <stdint.h>
#include <stdbool.h>

/* Audio buffer parameters */
#define AUDIO_BLOCK_SIZE        256     /* Samples per channel per processing block */
#define AUDIO_SAMPLE_RATE_HZ    48000
#define AUDIO_CHANNELS_STEREO   2

/**
 * @brief Audio input source selection
 */
typedef enum {
    AUDIO_INPUT_CODEC = 0,      /* I2S from AC328904 codec ADC */
    AUDIO_INPUT_MIC2  = 1,      /* Analog from MIC 2 Click via ADC */
    AUDIO_INPUT_TEST_TONE = 2   /* Internal sine wave generator */
} audio_input_source_t;

/**
 * @brief DSP effect enable flags
 */
typedef struct {
    bool noiseReduction;        /* Simple noise gate */
    bool equalizerEnabled;      /* 5-band parametric EQ */
    bool bassBoost;             /* Low-frequency shelf boost */
    bool trebleBoost;           /* High-frequency shelf boost */
    bool echoEnabled;           /* Delay-based echo */
    bool reverbEnabled;         /* Simple reverb */
} audio_effects_t;

/**
 * @brief Audio pipeline configuration
 */
typedef struct {
    audio_input_source_t inputSource;
    audio_effects_t effects;
    uint8_t volume;             /* 0-255 */
    uint8_t bassLevel;          /* 0-10 */
    uint8_t trebleLevel;        /* 0-10 */
    uint8_t eqBand[5];          /* 5-band EQ levels, 0-10 each */
    uint8_t echoDelay;          /* Echo delay in ms (10-500) */
    uint8_t echoDecay;          /* Echo decay 0-10 */
    uint16_t testToneFreq;      /* Test tone frequency in Hz */
} audio_config_t;

/**
 * @brief Initialize the audio processing pipeline
 */
void AudioPipeline_Initialize(void);

/**
 * @brief Start audio streaming
 */
void AudioPipeline_Start(void);

/**
 * @brief Stop audio streaming
 */
void AudioPipeline_Stop(void);

/**
 * @brief Process one block of audio samples (called from DMA ISR context)
 * @param input Pointer to input buffer (stereo interleaved, 16-bit signed)
 * @param output Pointer to output buffer
 * @param blockSize Number of stereo sample pairs
 */
void AudioPipeline_ProcessBlock(int16_t *input, int16_t *output, uint16_t blockSize);

/**
 * @brief Get current audio configuration
 */
audio_config_t* AudioPipeline_GetConfig(void);

/**
 * @brief Set input source
 */
void AudioPipeline_SetInputSource(audio_input_source_t source);

/**
 * @brief Enable/disable individual effects
 */
void AudioPipeline_SetEffects(audio_effects_t *effects);

/**
 * @brief Stream a block of raw samples over UART for visualization
 * @param buffer Audio buffer to stream
 * @param count Number of samples to stream
 */
void AudioPipeline_StreamSamples(int16_t *buffer, uint16_t count);

#endif /* AUDIO_PIPELINE_H */
