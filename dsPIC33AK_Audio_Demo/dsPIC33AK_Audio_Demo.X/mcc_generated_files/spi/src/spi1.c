/**
 * SPI1 Generated Driver Source File (I2S Audio Mode)
 * 
 * @file      spi1.c
 * @ingroup   spidriver
 * @brief     SPI1 in I2S master mode for AC328904 Audio Codec Board
 * @version   PLIB Version 1.0.0
 * @skipline  Device : dsPIC33AK512MPS512
 *
 * Configuration:
 *   - I2S Master mode
 *   - 16-bit audio data
 *   - BCLK generated from Standard Peripheral Clock (100 MHz)
 *   - BCLK = 100 MHz / (2 * (BRG+1)) -> BRG = 31 for ~1.5625 MHz
 *   - LRCK = BCLK / 32 = ~48.8 kHz (close to 48 kHz)
 *
 * dsPIC33A SPI register differences from dsPIC33F/E:
 *   - SPIxCON is 32-bit (was SPIxCON1 + SPIxCON2 on dsPIC33F/E)
 *   - SPIxCON.AUDEN bit enables audio/I2S mode
 *   - SPIxCON.AUDMOD<1:0> selects I2S, Left-justified, Right-justified, PCM
 *   - SPIxBUF is 32-bit accessible
 *   - SPIxSTAT has different bit positions
 *   - SPIxBRG is 32-bit
 */

#include <xc.h>
#include <stdint.h>
#include "../spi1.h"

/* SPI1 clock source: Standard Speed Peripheral Clock = 100 MHz */
#define SPI1_CLOCK          100000000UL
#define SPI1_TARGET_BCLK    1536000UL   /* 48kHz * 16bits * 2ch */

void SPI1_Initialize(void)
{
    /* Disable SPI1 before configuration */
    SPI1CONbits.ON = 0;

    /* 
     * SPI1CON Configuration for I2S Master:
     *   MSTEN = 1   : Master mode
     *   CKP   = 0   : Clock idle low
     *   CKE   = 1   : Data changes on falling edge, sampled on rising
     *   SMP   = 0   : Input sampled at middle of data output time
     *   MODE  = 0b00: 16-bit mode
     *   AUDEN = 1   : Audio protocol enable
     *   AUDMOD= 0b00: I2S mode
     *   FRMEN = 1   : Framed SPI (generates LRCK on SS pin)
     *   FRMSYNC = 0 : Frame sync pulse output (master)
     *   FRMPOL = 0  : Frame sync active low (I2S standard)
     *   DISSDO = 0  : SDO enabled
     *   DISSDI = 0  : SDI enabled
     *   ENHBUF = 1  : Enhanced buffer mode (FIFO)
     *   STXISEL= 0b01: Interrupt when TX buffer is not full
     *   SRXISEL= 0b01: Interrupt when RX buffer is not empty
     */
    SPI1CON = 0x00000000UL;
    SPI1CONbits.MSTEN = 1;     /* Master mode */
    SPI1CONbits.CKE = 1;       /* Data changes on active->idle transition */
    SPI1CONbits.ENHBUF = 1;    /* Enhanced buffer (FIFO) */
    SPI1CONbits.AUDEN = 1;     /* Audio protocol enable (I2S) */
    SPI1CONbits.AUDMOD = 0;    /* I2S mode */
    SPI1CONbits.FRMEN = 1;     /* Framed SPI for LRCK generation */
    SPI1CONbits.STXISEL = 1;   /* TX interrupt when not full */
    SPI1CONbits.SRXISEL = 1;   /* RX interrupt when not empty */

    /*
     * SPI1STAT: Clear error flags
     */
    SPI1STAT = 0x00000000UL;

    /*
     * SPI1BRG: Baud Rate Generator
     * BCLK = FPERI / (2 * (BRG + 1))
     * For ~1.536 MHz: BRG = 100MHz / (2 * 1.536MHz) - 1 = ~31.5 -> 31
     * Actual BCLK = 100MHz / (2 * 32) = 1.5625 MHz
     * Actual Fs = 1.5625MHz / 32 = 48.828 kHz (within 2% of 48 kHz)
     */
    SPI1BRG = 31UL;

    /* Note: SPI1 is NOT enabled here. 
     * Call SPI1_Enable() after DMA is configured. */
}

void SPI1_Deinitialize(void)
{
    SPI1CONbits.ON = 0;
    SPI1CON = 0x00000000UL;
    SPI1STAT = 0x00000000UL;
    SPI1BRG = 0x00000000UL;
}

void SPI1_Enable(void)
{
    SPI1CONbits.ON = 1;
}

void SPI1_Disable(void)
{
    SPI1CONbits.ON = 0;
}

void SPI1_Write(uint16_t data)
{
    while(SPI1STATbits.TXFULL) {}
    SPI1BUF = (uint32_t)data;
}

uint16_t SPI1_Read(void)
{
    while(SPI1STATbits.RXBE) {}
    return (uint16_t)(SPI1BUF & 0xFFFF);
}

bool SPI1_IsTxReady(void)
{
    return !SPI1STATbits.TXFULL;
}

bool SPI1_IsRxReady(void)
{
    return !SPI1STATbits.RXBE;
}

volatile uint16_t* SPI1_TxBufferAddressGet(void)
{
    return (volatile uint16_t*)&SPI1BUF;
}

volatile uint16_t* SPI1_RxBufferAddressGet(void)
{
    return (volatile uint16_t*)&SPI1BUF;
}
