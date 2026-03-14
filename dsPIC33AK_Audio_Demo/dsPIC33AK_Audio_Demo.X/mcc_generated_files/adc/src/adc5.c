/**
 * ADC5 Generated Driver Source File
 * 
 * @file      adc5.c
 * @ingroup   adcdriver
 * @brief     ADC5 multi-core ADC configuration for dsPIC33AK512MPS512 Audio Demo
 * @version   PLIB Version 1.0.0
 * @skipline  Device : dsPIC33AK512MPS512
 *
 * The dsPIC33AK512MPS512 uses a multi-core SAR ADC (ADC5) with 
 * per-channel configuration. This is fundamentally different from
 * the traditional ADC1CON/ADC1BUF pattern on dsPIC33F/E.
 *
 * Multi-core ADC architecture:
 *   - Module-level control: AD5CON (ON, ADRDY, CALREQ, CALRDY)
 *   - Per-channel config:   AD5CH0CON1 (trigger, mode, pin, sample time)
 *   - Per-channel result:   AD5CH0DATA
 *   - Per-channel status:   AD5STATbits.CH0RDY
 *   - Software trigger:     AD5SWTRGbits.CH0TRG
 *
 * Channel 0 configuration for MIC 2 Click:
 *   - Input:  AD5AN0 (maps to RA0 on mikroBUS socket)
 *   - Mode:   Single-ended
 *   - Trigger: Software or Timer-based for 48 kHz sampling
 *   - Format: Signed fractional (left-justified) for DSP compatibility
 *
 * Interrupt: IFS7bits.AD5CH0IF / IEC7bits.AD5CH0IE / _AD5CH0Interrupt
 */

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "../adc5.h"

/* Callback for Channel 0 conversion complete */
static void (*adc5Ch0Handler)(void) = NULL;

void ADC5_Initialize(void)
{
    /* ---- Step 1: Power up ADC5 module ---- */
    AD5CONbits.ON = 1;

    /* Wait for ADC to be ready */
#ifndef __MPLAB_DEBUGGER_SIMULATOR
    while(AD5CONbits.ADRDY == 0U) {}
#endif

    /* ---- Step 2: Run calibration ---- */
    AD5CONbits.CALREQ = 1;
#ifndef __MPLAB_DEBUGGER_SIMULATOR
    while(AD5CONbits.CALRDY == 0U) {}
#endif

    /* ---- Step 3: Configure Channel 0 for MIC 2 Click ---- */
    /*
     * AD5CH0CON1:
     *   PINSEL  = 0   : AD5AN0 (RA0) as positive input
     *   NINSEL  = 0   : VREFL as negative input (single-ended)
     *   DIFF    = 0   : Single-ended mode
     *   LEFT    = 1   : Left-justify result (Q15-compatible signed fractional)
     *   SAMC    = 31  : 31 TAD sampling time (adequate for MIC impedance)
     *   MODE    = 0   : Single sample mode
     *   TRG1SRC = 0   : Software trigger (use AD5SWTRGbits.CH0TRG)
     *   EIEN    = 1   : Early interrupt enable (interrupt on conversion done)
     *
     * NOTE: For DMA-triggered continuous sampling at 48 kHz, TRG1SRC
     * should be changed to a timer source. The specific code depends on
     * which timer/PWM is used. For now, software trigger is used for
     * initial testing. See comment below for timer trigger setup.
     */
    AD5CH0CON1 = 0x00000000UL;
    AD5CH0CON1bits.PINSEL = 0;     /* AD5AN0 (RA0) */
    AD5CH0CON1bits.NINSEL = 0;     /* VREFL */
    AD5CH0CON1bits.DIFF = 0;       /* Single-ended */
    AD5CH0CON1bits.LEFT = 1;       /* Left-justified (signed fractional) */
    AD5CH0CON1bits.SAMC = 31;      /* 31 TAD sample time */
    AD5CH0CON1bits.MODE = 0;       /* Single sample */
    AD5CH0CON1bits.TRG1SRC = 0;    /* Software trigger */
    AD5CH0CON1bits.EIEN = 1;       /* Early interrupt enable */

    /*
     * AD5CH0CON2:
     *   CMPMOD   = 0  : Comparator disabled
     *   ACCNUM   = 0  : No accumulation
     *   ACCBRST  = 0  : No accumulator burst
     *   ACCRO    = 0  : Accumulator read-only disabled
     */
    AD5CH0CON2 = 0x00000000UL;

    /* ---- Step 4: Configure interrupt ---- */
    IFS7bits.AD5CH0IF = 0;          /* Clear interrupt flag */
    IPC30bits.AD5CH0IP = 4;         /* Priority 4 (moderate) */
    IEC7bits.AD5CH0IE = 1;          /* Enable Channel 0 interrupt */

    /*
     * NOTE: For continuous 48 kHz sampling with DMA, you would:
     *   1. Configure a timer (e.g., SCCP/TMR) to trigger at 48 kHz
     *   2. Set AD5CH0CON1bits.TRG1SRC to the timer trigger source number
     *   3. The DMA trigger source for ADC5 needs to be looked up in the
     *      device datasheet DMA trigger source table.
     *   4. DMA would read from AD5CH0DATA instead of the old ADC1BUF0.
     *
     * Timer trigger source codes are device-specific. Refer to
     * "DMA Trigger Source Selection" table in the datasheet.
     */
}

void ADC5_Deinitialize(void)
{
    IEC7bits.AD5CH0IE = 0;
    AD5CONbits.ON = 0;
}

void ADC5_Enable(void)
{
    AD5CONbits.ON = 1;
#ifndef __MPLAB_DEBUGGER_SIMULATOR
    while(AD5CONbits.ADRDY == 0U) {}
#endif
}

void ADC5_Disable(void)
{
    AD5CONbits.ON = 0;
}

void ADC5_Channel0SoftwareTrigger(void)
{
    AD5SWTRGbits.CH0TRG = 1;
}

bool ADC5_Channel0IsReady(void)
{
    return AD5STATbits.CH0RDY;
}

int16_t ADC5_Channel0ResultGet(void)
{
    /* AD5CH0DATA contains left-justified 12-bit result in signed fractional
     * format when LEFT=1. Read as signed 16-bit for Q15 DSP compatibility. */
    return (int16_t)(AD5CH0DATA & 0xFFFF);
}

volatile uint16_t* ADC5_Channel0DataAddressGet(void)
{
    return (volatile uint16_t*)&AD5CH0DATA;
}

void ADC5_Channel0CallbackRegister(void (*handler)(void))
{
    adc5Ch0Handler = handler;
}

/**
 * @brief ADC5 Channel 0 Interrupt Service Routine
 *
 * Fires when Channel 0 conversion is complete.
 * Calls registered callback and clears interrupt flag.
 */
void __attribute__((interrupt, no_auto_psv)) _AD5CH0Interrupt(void)
{
    if(adc5Ch0Handler != NULL)
    {
        adc5Ch0Handler();
    }

    IFS7bits.AD5CH0IF = 0;
}
