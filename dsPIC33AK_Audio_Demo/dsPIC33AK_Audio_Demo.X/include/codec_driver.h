/**
 * Audio Codec Driver Header
 * 
 * @file      codec_driver.h
 * @brief     AK4642 codec control API for AC328904 Audio Codec Board
 */

#ifndef CODEC_DRIVER_H
#define CODEC_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Audio input source selection
 */
typedef enum {
    CODEC_INPUT_LINE = 0,   /* Line input (from AC328904 line-in jack) */
    CODEC_INPUT_MIC  = 1    /* Microphone input */
} codec_input_t;

/**
 * @brief Initialize the AK4642 codec hardware
 * Must be called after I2C1_Initialize() and PINS_Initialize()
 */
void CODEC_Initialize(void);

/**
 * @brief Shut down the codec
 */
void CODEC_Deinitialize(void);

/**
 * @brief Set the output volume
 * @param volume 0 (mute) to 255 (maximum)
 */
void CODEC_SetVolume(uint8_t volume);

/**
 * @brief Get current volume setting
 * @return Volume 0-255
 */
uint8_t CODEC_GetVolume(void);

/**
 * @brief Enable or disable soft mute
 * @param mute true to mute, false to unmute
 */
void CODEC_SetMute(bool mute);

/**
 * @brief Select the audio input source
 * @param source CODEC_INPUT_LINE or CODEC_INPUT_MIC
 */
void CODEC_SetInputSource(codec_input_t source);

/**
 * @brief Set the sample rate
 * @param sampleRate Sample rate in Hz (8000, 16000, 44100, 48000, etc.)
 */
void CODEC_SetSampleRate(uint32_t sampleRate);

#endif /* CODEC_DRIVER_H */
