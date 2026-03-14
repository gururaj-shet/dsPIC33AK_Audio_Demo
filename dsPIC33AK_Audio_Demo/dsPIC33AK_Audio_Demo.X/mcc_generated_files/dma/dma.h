/**
 * DMA Generated Driver Header File
 * 
 * @file      dma.h
 * @ingroup   dmadriver
 * @brief     DMA configuration for audio double-buffering on dsPIC33AK512MPS512
 * @version   PLIB Version 1.0.0
 * @skipline  Device : dsPIC33AK512MPS512
 *
 * DMA channel assignments:
 *   Channel 0: SPI1 TX (Audio output to codec DAC)
 *   Channel 1: SPI1 RX (Audio input from codec ADC)
 *   Channel 2: ADC1    (Microphone analog input via ADC)
 *
 * All channels use ping-pong (double) buffering for continuous
 * audio streaming without gaps.
 *
 * dsPIC33A DMA differences from dsPIC33F/E:
 *   - DMA channels use DMACHx registers (32-bit) instead of DMAx registers
 *   - Source/destination addresses are DMASRCx/DMADSTx (32-bit)
 *   - Count register is DMACNTx (32-bit)
 *   - Trigger source selection via DMACHxbits.TRGSRC
 *   - No DMAPAD register; peripheral addresses set directly
 */

#ifndef DMA_H
#define DMA_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

/* Audio buffer size: 256 stereo samples per half-buffer */
#define DMA_AUDIO_BUFFER_SIZE       512     /* 256 samples * 2 channels */
#define DMA_AUDIO_HALF_BUFFER_SIZE  256     /* Half-buffer for processing */

/**
 * @brief Initialize all DMA channels for audio processing
 */
void DMA_Initialize(void);

/**
 * @brief Deinitialize DMA
 */
void DMA_Deinitialize(void);

/**
 * @brief Enable DMA Channel 0 (SPI1 TX)
 */
void DMA_Channel0Enable(void);

/**
 * @brief Enable DMA Channel 1 (SPI1 RX)
 */
void DMA_Channel1Enable(void);

/**
 * @brief Enable DMA Channel 2 (ADC)
 */
void DMA_Channel2Enable(void);

/**
 * @brief Disable all DMA channels
 */
void DMA_DisableAll(void);

/**
 * @brief Register callback for DMA Channel 1 complete (RX buffer ready)
 * @param handler Function pointer to callback
 */
void DMA_Channel1TransferDoneCallbackRegister(void (*handler)(void));

/**
 * @brief Register callback for DMA Channel 2 complete (ADC buffer ready)
 * @param handler Function pointer to callback
 */
void DMA_Channel2TransferDoneCallbackRegister(void (*handler)(void));

/**
 * @brief Get pointer to current active TX buffer (ping or pong)
 */
int16_t* DMA_AudioTxBufferGet(void);

/**
 * @brief Get pointer to last completed RX buffer (for processing)
 */
int16_t* DMA_AudioRxBufferGet(void);

/**
 * @brief Get pointer to last completed ADC buffer
 */
int16_t* DMA_AdcBufferGet(void);

#endif /* DMA_H */
