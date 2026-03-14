/**
 * MIC 2 Click Driver Header
 * 
 * @file      mic_driver.h
 * @brief     MikroElektronika MIC 2 Click board control API
 */

#ifndef MIC_DRIVER_H
#define MIC_DRIVER_H

#include <stdint.h>

/**
 * @brief Initialize MIC 2 Click board (set default gain)
 */
void MIC2_Initialize(void);

/**
 * @brief Set microphone gain via digital potentiometer
 * @param gain 0 (minimum) to 255 (maximum gain)
 */
void MIC2_SetGain(uint8_t gain);

/**
 * @brief Get current microphone gain setting
 * @return Current gain 0-255
 */
uint8_t MIC2_GetGain(void);

/**
 * @brief Read a single audio sample from the microphone (polling mode)
 * @return Signed 16-bit audio sample
 */
int16_t MIC2_ReadSample(void);

#endif /* MIC_DRIVER_H */
