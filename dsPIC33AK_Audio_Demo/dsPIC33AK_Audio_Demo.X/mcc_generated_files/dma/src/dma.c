/**
 * DMA Generated Driver Source File
 * 
 * @file      dma.c
 * @ingroup   dmadriver
 * @brief     DMA channels for audio double-buffering on dsPIC33AK512MPS512
 * @version   PLIB Version 1.0.0
 * @skipline  Device : dsPIC33AK512MPS512
 *
 * Double-buffering strategy:
 *   - Two buffers (A and B) for each direction
 *   - While DMA fills buffer A, DSP processes buffer B
 *   - On DMA complete interrupt, buffers swap
 *   - This ensures zero-copy, zero-gap audio streaming
 *
 * dsPIC33A DMA architecture notes:
 *   - Up to 8 DMA channels
 *   - Each channel: DMACHn, DMASRCn, DMADSTn, DMACNTn registers (all 32-bit)
 *   - Ping-pong mode: DMACHnbits.PPMODE = 1
 *   - Trigger sources are device-specific numeric codes
 *   - DMA can access entire data space (no DPSRAM limitation like dsPIC33F)
 */

#include <xc.h>
#include <stdint.h>
#include <string.h>
#include "../dma.h"
#include "../../spi/spi1.h"

/* ---- Audio DMA Buffers (double-buffered) ---- */
/* These must be in DMA-accessible RAM. On dsPIC33A, all RAM is DMA-accessible. */
static int16_t __attribute__((aligned(4))) dmaTxBufferA[DMA_AUDIO_BUFFER_SIZE];
static int16_t __attribute__((aligned(4))) dmaTxBufferB[DMA_AUDIO_BUFFER_SIZE];
static int16_t __attribute__((aligned(4))) dmaRxBufferA[DMA_AUDIO_BUFFER_SIZE];
static int16_t __attribute__((aligned(4))) dmaRxBufferB[DMA_AUDIO_BUFFER_SIZE];

/* ADC DMA buffers (mono microphone) */
static int16_t __attribute__((aligned(4))) dmaAdcBufferA[DMA_AUDIO_HALF_BUFFER_SIZE];
static int16_t __attribute__((aligned(4))) dmaAdcBufferB[DMA_AUDIO_HALF_BUFFER_SIZE];

/* Buffer tracking */
static volatile uint8_t txActiveBuffer = 0;     /* 0 = A active, 1 = B active */
static volatile uint8_t rxActiveBuffer = 0;
static volatile uint8_t adcActiveBuffer = 0;

/* Callback function pointers */
static void (*dmaChannel1DoneHandler)(void) = NULL;
static void (*dmaChannel2DoneHandler)(void) = NULL;

/* DMA trigger source definitions for dsPIC33AK512MPS512
 * NOTE: These trigger source codes are device-specific and must be verified
 * against the "DMA Trigger Source Selection" table in the dsPIC33AK512MPS512
 * datasheet (DS70005611). The values below are placeholder estimates.
 * SPI trigger codes are typically in the 0x0A-0x0B range for SPI1.
 * ADC5 trigger code needs verification -- it may differ from ADC1's code.
 */
#define DMA_TRGSRC_SPI1_TX      0x0AU   /* SPI1 Transfer Done (VERIFY vs datasheet) */
#define DMA_TRGSRC_SPI1_RX      0x0BU   /* SPI1 Receive Done  (VERIFY vs datasheet) */
#define DMA_TRGSRC_ADC5_CH0     0x15U   /* ADC5 Channel 0 Done (VERIFY vs datasheet) */

void DMA_Initialize(void)
{
    /* Clear all buffers */
    memset(dmaTxBufferA, 0, sizeof(dmaTxBufferA));
    memset(dmaTxBufferB, 0, sizeof(dmaTxBufferB));
    memset(dmaRxBufferA, 0, sizeof(dmaRxBufferA));
    memset(dmaRxBufferB, 0, sizeof(dmaRxBufferB));
    memset(dmaAdcBufferA, 0, sizeof(dmaAdcBufferA));
    memset(dmaAdcBufferB, 0, sizeof(dmaAdcBufferB));

    /*
     * ---- DMA Channel 0: SPI1 TX (Audio Output to Codec DAC) ----
     * Direction: Memory -> SPI1BUF
     * Mode: Continuous, Ping-Pong
     * Size: 16-bit transfers
     * Trigger: SPI1 TX ready
     */
    DMACH0 = 0x00000000UL;
    DMASRC0 = (uint32_t)dmaTxBufferA;          /* Source: TX buffer A start */
    DMADST0 = (uint32_t)&SPI1BUF;              /* Destination: SPI1 TX register */
    DMACNT0 = DMA_AUDIO_BUFFER_SIZE - 1;        /* Transfer count (N-1) */
    DMACH0bits.TRGSRC = DMA_TRGSRC_SPI1_TX;    /* Trigger: SPI1 TX */
    DMACH0bits.SIZE = 1;                         /* 16-bit transfers */
    DMACH0bits.DIR = 1;                          /* Read from RAM, write to peripheral */
    DMACH0bits.SAMODE = 1;                       /* Source address increment */
    DMACH0bits.DAMODE = 0;                       /* Destination address fixed */
    DMACH0bits.PPMODE = 1;                       /* Ping-pong mode */
    DMACH0bits.RELOAD = 1;                       /* Auto-reload source address */

    /*
     * ---- DMA Channel 1: SPI1 RX (Audio Input from Codec ADC) ----
     * Direction: SPI1BUF -> Memory
     * Mode: Continuous, Ping-Pong
     * Size: 16-bit transfers
     * Trigger: SPI1 RX ready
     */
    DMACH1 = 0x00000000UL;
    DMASRC1 = (uint32_t)&SPI1BUF;              /* Source: SPI1 RX register */
    DMADST1 = (uint32_t)dmaRxBufferA;          /* Destination: RX buffer A */
    DMACNT1 = DMA_AUDIO_BUFFER_SIZE - 1;
    DMACH1bits.TRGSRC = DMA_TRGSRC_SPI1_RX;
    DMACH1bits.SIZE = 1;                         /* 16-bit */
    DMACH1bits.DIR = 0;                          /* Read from peripheral, write to RAM */
    DMACH1bits.SAMODE = 0;                       /* Source address fixed (SPI reg) */
    DMACH1bits.DAMODE = 1;                       /* Destination address increment */
    DMACH1bits.PPMODE = 1;                       /* Ping-pong */
    DMACH1bits.RELOAD = 1;

    /* Enable DMA Ch1 transfer complete interrupt */
    IFS0bits.DMA1IF = 0;
    IEC0bits.DMA1IE = 1;

    /*
     * ---- DMA Channel 2: ADC5 (Microphone Input via MIC 2 Click) ----
     * Direction: AD5CH0DATA -> Memory
     * Mode: Continuous, Ping-Pong
     * Size: 16-bit transfers
     * Trigger: ADC5 Channel 0 conversion complete
     *
     * NOTE: The dsPIC33AK512MPS512 uses multi-core ADC5 with per-channel
     * data registers (AD5CH0DATA). This replaces the old ADC1BUF0 pattern.
     */
    DMACH2 = 0x00000000UL;
    DMASRC2 = (uint32_t)&AD5CH0DATA;           /* Source: ADC5 Ch0 result register */
    DMADST2 = (uint32_t)dmaAdcBufferA;
    DMACNT2 = DMA_AUDIO_HALF_BUFFER_SIZE - 1;
    DMACH2bits.TRGSRC = DMA_TRGSRC_ADC5_CH0;
    DMACH2bits.SIZE = 1;
    DMACH2bits.DIR = 0;
    DMACH2bits.SAMODE = 0;
    DMACH2bits.DAMODE = 1;
    DMACH2bits.PPMODE = 1;
    DMACH2bits.RELOAD = 1;

    /* Enable DMA Ch2 transfer complete interrupt */
    IFS0bits.DMA2IF = 0;
    IEC0bits.DMA2IE = 1;

    /* Set ping-pong buffer B addresses */
    /* On dsPIC33A, ping-pong alternate addresses are set through
     * the secondary source/destination registers or by ISR reload.
     * The DMA controller alternates between primary and secondary
     * address registers automatically in ping-pong mode.
     */
}

void DMA_Deinitialize(void)
{
    DMACH0bits.CHEN = 0;
    DMACH1bits.CHEN = 0;
    DMACH2bits.CHEN = 0;
}

void DMA_Channel0Enable(void)
{
    DMACH0bits.CHEN = 1;
}

void DMA_Channel1Enable(void)
{
    DMACH1bits.CHEN = 1;
}

void DMA_Channel2Enable(void)
{
    DMACH2bits.CHEN = 1;
}

void DMA_DisableAll(void)
{
    DMACH0bits.CHEN = 0;
    DMACH1bits.CHEN = 0;
    DMACH2bits.CHEN = 0;
}

void DMA_Channel1TransferDoneCallbackRegister(void (*handler)(void))
{
    dmaChannel1DoneHandler = handler;
}

void DMA_Channel2TransferDoneCallbackRegister(void (*handler)(void))
{
    dmaChannel2DoneHandler = handler;
}

int16_t* DMA_AudioTxBufferGet(void)
{
    /* Return the buffer NOT currently used by DMA (safe to write) */
    return (txActiveBuffer == 0) ? dmaTxBufferB : dmaTxBufferA;
}

int16_t* DMA_AudioRxBufferGet(void)
{
    /* Return the last completed buffer (safe to read/process) */
    return (rxActiveBuffer == 0) ? dmaRxBufferB : dmaRxBufferA;
}

int16_t* DMA_AdcBufferGet(void)
{
    return (adcActiveBuffer == 0) ? dmaAdcBufferB : dmaAdcBufferA;
}

/**
 * DMA Channel 1 Interrupt: SPI1 RX complete (codec audio input ready)
 * 
 * This ISR fires when a full buffer of audio samples has been received
 * from the codec. It swaps the active buffer and optionally calls
 * the registered callback for DSP processing.
 */
void __attribute__((interrupt, no_auto_psv)) _DMA1Interrupt(void)
{
    /* Toggle active buffer index */
    rxActiveBuffer ^= 1;
    txActiveBuffer ^= 1;

    /* Reload DMA source/destination for next ping-pong cycle */
    if(rxActiveBuffer == 0)
    {
        DMADST1 = (uint32_t)dmaRxBufferA;
        DMASRC0 = (uint32_t)dmaTxBufferA;
    }
    else
    {
        DMADST1 = (uint32_t)dmaRxBufferB;
        DMASRC0 = (uint32_t)dmaTxBufferB;
    }

    /* Call processing callback */
    if(dmaChannel1DoneHandler != NULL)
    {
        dmaChannel1DoneHandler();
    }

    IFS0bits.DMA1IF = 0;
}

/**
 * DMA Channel 2 Interrupt: ADC buffer complete (microphone data ready)
 */
void __attribute__((interrupt, no_auto_psv)) _DMA2Interrupt(void)
{
    adcActiveBuffer ^= 1;

    if(adcActiveBuffer == 0)
    {
        DMADST2 = (uint32_t)dmaAdcBufferA;
    }
    else
    {
        DMADST2 = (uint32_t)dmaAdcBufferB;
    }

    if(dmaChannel2DoneHandler != NULL)
    {
        dmaChannel2DoneHandler();
    }

    IFS0bits.DMA2IF = 0;
}
