/**
 * SPI1 Generated Driver Header File (I2S Audio Mode)
 * 
 * @file      spi1.h
 * @ingroup   spidriver
 * @brief     SPI1 configured in I2S master mode for audio codec interface
 * @version   PLIB Version 1.0.0
 * @skipline  Device : dsPIC33AK512MPS512
 *
 * SPI1 operates in I2S audio mode:
 *   - Master mode (generates BCLK and LRCK)
 *   - 16-bit word, stereo (32-bit frame = L + R)
 *   - Sample rate: 48 kHz
 *   - BCLK = 48000 * 16 * 2 = 1.536 MHz
 *   - Full duplex (simultaneous TX and RX via DMA)
 *
 * Ported from dsPIC33F/E: The dsPIC33A SPI module uses SPIxCON (32-bit)
 * instead of SPIxCON1/SPIxCON2 (16-bit). Audio mode bits are in SPIxCON.
 */

#ifndef SPI1_H
#define SPI1_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

/* Audio frame size definitions */
#define SPI1_AUDIO_FRAME_SAMPLES    256     /* Samples per channel per frame */
#define SPI1_AUDIO_CHANNELS         2       /* Stereo */
#define SPI1_AUDIO_FRAME_SIZE       (SPI1_AUDIO_FRAME_SAMPLES * SPI1_AUDIO_CHANNELS)

/**
 * @brief Initialize SPI1 in I2S audio master mode
 */
void SPI1_Initialize(void);

/**
 * @brief Deinitialize SPI1
 */
void SPI1_Deinitialize(void);

/**
 * @brief Enable SPI1 module
 */
void SPI1_Enable(void);

/**
 * @brief Disable SPI1 module
 */
void SPI1_Disable(void);

/**
 * @brief Write a 16-bit sample to the SPI1 transmit buffer
 * @param data 16-bit audio sample
 */
void SPI1_Write(uint16_t data);

/**
 * @brief Read a 16-bit sample from the SPI1 receive buffer
 * @return 16-bit audio sample
 */
uint16_t SPI1_Read(void);

/**
 * @brief Check if transmit buffer is not full
 */
bool SPI1_IsTxReady(void);

/**
 * @brief Check if receive buffer has data
 */
bool SPI1_IsRxReady(void);

/**
 * @brief Get SPI1 TX buffer register address (for DMA)
 */
volatile uint16_t* SPI1_TxBufferAddressGet(void);

/**
 * @brief Get SPI1 RX buffer register address (for DMA)
 */
volatile uint16_t* SPI1_RxBufferAddressGet(void);

#endif /* SPI1_H */
