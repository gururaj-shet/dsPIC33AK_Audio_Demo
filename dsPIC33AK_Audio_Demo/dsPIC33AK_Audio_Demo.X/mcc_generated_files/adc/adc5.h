/**
 * ADC5 Generated Driver Header File
 * 
 * @file      adc5.h
 * @ingroup   adcdriver
 * @brief     ADC5 multi-core ADC for microphone input on dsPIC33AK512MPS512
 * @version   PLIB Version 1.0.0
 * @skipline  Device : dsPIC33AK512MPS512
 *
 * The dsPIC33AK512MPS512 uses a multi-core ADC architecture (ADC5).
 * Each ADC channel has independent configuration registers:
 *   AD5CHxCON1/CON2 per channel, AD5CHxDATA for results.
 *
 * Channel 0 is configured for MIC 2 Click analog input on AD5AN0 (RA0).
 *
 * Register naming convention:
 *   AD5CON       - Module-level control
 *   AD5STAT      - Channel-ready status
 *   AD5SWTRG     - Software trigger
 *   AD5CH0CON1   - Channel 0 configuration
 *   AD5CH0DATA   - Channel 0 result
 *
 * Interrupt:
 *   IFS7bits.AD5CH0IF, IEC7bits.AD5CH0IE, IPC30bits.AD5CH0IP
 *   ISR: _AD5CH0Interrupt
 */

#ifndef ADC5_H
#define ADC5_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initialize ADC5 module and configure Channel 0 for microphone sampling
 *
 * Performs:
 *   1. Module power-up (AD5CONbits.ON)
 *   2. Wait for ready (AD5CONbits.ADRDY)
 *   3. Calibration (AD5CONbits.CALREQ, wait for CALRDY)
 *   4. Configure Channel 0 for AD5AN0 input
 */
void ADC5_Initialize(void);

/**
 * @brief Deinitialize ADC5 module
 */
void ADC5_Deinitialize(void);

/**
 * @brief Enable ADC5 module
 */
void ADC5_Enable(void);

/**
 * @brief Disable ADC5 module
 */
void ADC5_Disable(void);

/**
 * @brief Trigger a software conversion on Channel 0
 */
void ADC5_Channel0SoftwareTrigger(void);

/**
 * @brief Check if Channel 0 conversion result is ready
 * @return true if result available
 */
bool ADC5_Channel0IsReady(void);

/**
 * @brief Read Channel 0 conversion result as signed 16-bit
 * @return Signed 16-bit sample (centered around 0)
 */
int16_t ADC5_Channel0ResultGet(void);

/**
 * @brief Get Channel 0 data register address (for DMA)
 * @return Address of AD5CH0DATA
 */
volatile uint16_t* ADC5_Channel0DataAddressGet(void);

/**
 * @brief Register callback for Channel 0 conversion complete interrupt
 * @param handler Function pointer to callback
 */
void ADC5_Channel0CallbackRegister(void (*handler)(void));

#endif /* ADC5_H */
