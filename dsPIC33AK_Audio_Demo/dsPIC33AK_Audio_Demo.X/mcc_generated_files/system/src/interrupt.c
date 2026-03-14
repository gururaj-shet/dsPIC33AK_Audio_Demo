/**
 * INTERRUPT Generated Driver Source File
 * 
 * @file      interrupt.c
 * @ingroup   interruptdriver
 * @brief     Interrupt priority setup for dsPIC33AK512MPS512 Audio Demo
 * @version   PLIB Version 1.1.0
 * @skipline  Device : dsPIC33AK512MPS512
 *
 * Interrupt priorities for audio processing:
 *   DMA0 (SPI1 TX): Priority 6 (high - audio output must not underrun)
 *   DMA1 (SPI1 RX): Priority 6 (high - audio input must not overrun)
 *   DMA2 (ADC):     Priority 5 (microphone sampling)
 *   SPI1:           Priority 4 (I2S frame sync)
 *   TMR1:           Priority 1 (1 ms system tick)
 *   UART1 RX:       Priority 2 (CLI input)
 *   ADC:            Priority 3 (microphone ADC complete)
 *
 * Ported from dsPIC33F/E: dsPIC33A uses IPCx registers instead of
 * IPC0-IPC15 bit fields. Register addresses differ.
 */

#include "../interrupt.h"

void INTERRUPT_Initialize(void)
{
    /* TMR1: 1 ms system tick - lowest audio priority */
    IPC6bits.T1IP = 1;

    /* UART1 RX: CLI command processing */
    /* Note: UART1 RX interrupt priority set in UART driver if needed */

    /* DMA Channel 0 (SPI1 TX - audio output): High priority */
    IPC1bits.DMA0IP = 6;

    /* DMA Channel 1 (SPI1 RX - audio input): High priority */
    IPC1bits.DMA1IP = 6;

    /* DMA Channel 2 (ADC - microphone): Medium-high priority */
    IPC2bits.DMA2IP = 5;

    /* ADC complete interrupt */
    IPC8bits.ADCIP = 3;
}

void INTERRUPT_Deinitialize(void)
{
    /* Restore POR default values */
    IPC6bits.T1IP = 4;
    IPC1bits.DMA0IP = 4;
    IPC1bits.DMA1IP = 4;
    IPC2bits.DMA2IP = 4;
    IPC8bits.ADCIP = 4;
}
