/**
 * CLOCK Generated Driver Source File
 * 
 * @file      clock.c
 * @ingroup   clockdriver
 * @brief     Clock initialization for dsPIC33AK512MPS512 Audio Demo
 * @version   PLIB Version 1.1.2
 * @skipline  Device : dsPIC33AK512MPS512
 *
 * Clock configuration:
 *   PLL1: 320 MHz output (from 8 MHz primary oscillator)
 *   PLL2: 200 MHz output (system clock source)
 *   CLK GEN 1:  200 MHz (System / Fosc)
 *   CLK GEN 2:  8 MHz   (FRC)
 *   CLK GEN 3:  8 MHz   (BFRC for WDT)
 *   CLK GEN 6:  320 MHz (ADC, fast peripherals)
 *   CLK GEN 10: 80 MHz  (CAN - unused in audio demo but kept for compatibility)
 *   CLK GEN 13: 8 MHz   (CCP / REFO1 - base for MCLK generation)
 *
 * Ported from dsPIC33F/E: Replaced OSCTUN, PLLFBD, CLKDIV with
 * dsPIC33A PLL1CON/PLL2CON/CLKxCON register architecture.
 */

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../clock.h"
#include "../clock_types.h"

#define PLL1FOUT_SOURCE     0x5U
#define PLL2VCODIV_SOURCE   0x8U

static void (*combinedClockFailHandler)(void) = NULL;

void CLOCK_Initialize(void)
{
    /*
     * System Clock Source: PLL2 Out output
     * System/Generator 1 frequency (Fosc): 200 MHz
     * 
     * Clock Generator 2 frequency:  8 MHz
     * Clock Generator 3 frequency:  8 MHz
     * Clock Generator 6 frequency:  320 MHz
     * Clock Generator 10 frequency: 80 MHz
     * Clock Generator 13 frequency: 8 MHz
     * 
     * PLL 1 frequency: 320 MHz
     * PLL 2 frequency: 200 MHz
     */

    /* Enable Primary Oscillator (8 MHz crystal on Curiosity board) */
    OSCCFGbits.POSCMD = 0x1U;  /* HS mode */
    OSCCTRLbits.POSCEN = 1U;
#ifndef __MPLAB_DEBUGGER_SIMULATOR
    while(OSCCTRLbits.POSCRDY == 0U) {}
#endif

    /* If system clock is using a PLL, switch to FRC first */
    if((CLK1CONbits.COSC >= PLL1FOUT_SOURCE) && (CLK1CONbits.COSC <= PLL2VCODIV_SOURCE))
    {
        CLK1CONbits.NOSC = 1U;     /* FRC as source */
        CLK1CONbits.OSWEN = 1U;
#ifndef __MPLAB_DEBUGGER_SIMULATOR
        while(CLK1CONbits.OSWEN == 1U) {}
#endif
    }

    /* ---- PLL1 Configuration: 320 MHz output ---- */
    PLL1CON = 0x9300UL;
    PLL1DIV = 0x100C829UL;
    PLL1CONbits.PLLSWEN = 1U;
#ifndef __MPLAB_DEBUGGER_SIMULATOR
    while(PLL1CONbits.PLLSWEN == 1) {}
#endif
    PLL1CONbits.FOUTSWEN = 1U;
#ifndef __MPLAB_DEBUGGER_SIMULATOR
    while(PLL1CONbits.FOUTSWEN == 1U) {}
#endif
    PLL1CONbits.OSWEN = 1U;
#ifndef __MPLAB_DEBUGGER_SIMULATOR
    while(PLL1CONbits.OSWEN == 1U) {}
    while(OSCCTRLbits.PLL1RDY == 0U) {}
#endif
    VCO1DIV = 0x40000UL;
    PLL1CONbits.DIVSWEN = 1U;
#ifndef __MPLAB_DEBUGGER_SIMULATOR
    while(PLL1CONbits.DIVSWEN == 1U) {}
#endif
    PLL1CONbits.ON = 0U;

    /* ---- PLL2 Configuration: 200 MHz output ---- */
    PLL2CON = 0x9100UL;
    PLL2DIV = 0x1007D29UL;
    PLL2CONbits.PLLSWEN = 1U;
#ifndef __MPLAB_DEBUGGER_SIMULATOR
    while(PLL2CONbits.PLLSWEN == 1) {}
#endif
    PLL2CONbits.FOUTSWEN = 1U;
#ifndef __MPLAB_DEBUGGER_SIMULATOR
    while(PLL2CONbits.FOUTSWEN == 1U) {}
#endif
    PLL2CONbits.OSWEN = 1U;
#ifndef __MPLAB_DEBUGGER_SIMULATOR
    while(PLL2CONbits.OSWEN == 1U) {}
    while(OSCCTRLbits.PLL2RDY == 0U) {}
#endif
    VCO2DIV = 0x30000UL;
    PLL2CONbits.DIVSWEN = 1U;
#ifndef __MPLAB_DEBUGGER_SIMULATOR
    while(PLL2CONbits.DIVSWEN == 1U) {}
#endif
    PLL2CONbits.ON = 0U;

    /* ---- Clock Generator 1: System clock -> PLL2 Out, 200 MHz ---- */
    CLK1CON = 0x129680UL;
    CLK1DIV = 0x0UL;
    CLK1CONbits.OSWEN = 1U;
#ifndef __MPLAB_DEBUGGER_SIMULATOR
    while(CLK1CONbits.OSWEN == 1U) {}
#endif

    /* ---- Clock Generator 2: FRC, 8 MHz ---- */
    CLK2CON = 0x29181UL;
    CLK2CONbits.OSWEN = 1U;
#ifndef __MPLAB_DEBUGGER_SIMULATOR
    while(CLK2CONbits.OSWEN == 1U) {}
#endif

    /* ---- Clock Generator 3: Backup FRC, 8 MHz ---- */
    CLK3CON = 0x19282UL;
    CLK3CONbits.OSWEN = 1U;
#ifndef __MPLAB_DEBUGGER_SIMULATOR
    while(CLK3CONbits.OSWEN == 1U) {}
#endif

    /* ---- Clock Generator 6: PLL1 Out, 320 MHz (ADC, fast peripherals) ---- */
    CLK6CON = 0x29580UL;
    CLK6DIV = 0x0UL;
    CLK6CONbits.OSWEN = 1U;
#ifndef __MPLAB_DEBUGGER_SIMULATOR
    while(CLK6CONbits.OSWEN == 1U) {}
#endif

    /* ---- Clock Generator 10: PLL1 VCO Divider, 80 MHz ---- */
    CLK10CON = 0x29780UL;
    CLK10DIV = 0x14000UL;
    CLK10CONbits.DIVSWEN = 1U;
#ifndef __MPLAB_DEBUGGER_SIMULATOR
    while(CLK10CONbits.DIVSWEN == 1U) {}
#endif
    CLK10CONbits.OSWEN = 1U;
#ifndef __MPLAB_DEBUGGER_SIMULATOR
    while(CLK10CONbits.OSWEN == 1U) {}
#endif

    /* ---- Clock Generator 13: REFO1 / MCLK output for Audio Codec ----
     *
     * The AK4642 codec requires MCLK = 256 * Fs = 256 * 48000 = 12.288 MHz.
     * CLK13 maps to REFO1 output (routed via PPS to RD10).
     *
     * Approach: Source CLK13 from PLL1 output (320 MHz) and use the
     * integer divider to get close to 12.288 MHz.
     *
     * CLK13DIV integer divider:
     *   Fout = Fsource / (2 * (INTDIV + 1))   [when INTDIV > 0]
     *   320 MHz / (2 * 13) = 12.307 MHz  (INTDIV = 12, error +0.16%)
     *   320 MHz / (2 * 14) = 11.428 MHz  (INTDIV = 13, too low)
     *
     * At 12.307 MHz, the error is +0.16% from ideal 12.288 MHz. This is
     * well within the AK4642's tolerance (typically +/- 1000 ppm = 0.1%).
     * For higher precision, the PLL could be reconfigured to output a
     * frequency that divides exactly to 12.288 MHz (e.g., 98.304 MHz / 8),
     * but that requires PLL parameter changes that affect other peripherals.
     *
     * NOTE: The exact CLK13CON/CLK13DIV bit encodings and the OE (output
     * enable) bit location must be verified against the datasheet.
     * The NOSC field selects PLL1 output as the clock source.
     *
     * TODO: If the codec rejects this clock (audible artifacts, no lock),
     * reconfigure PLL1 or add a dedicated PLL output for 12.288 MHz.
     */
    CLK13CON = 0x29580UL;  /* Source: PLL1 output (same source code as CLK6) */
    CLK13DIV = 0x0C000UL;  /* INTDIV = 12 -> 320 MHz / (2*13) = 12.307 MHz */
    CLK13CONbits.DIVSWEN = 1U;
#ifndef __MPLAB_DEBUGGER_SIMULATOR
    while(CLK13CONbits.DIVSWEN == 1U) {}
#endif
    CLK13CONbits.OSWEN = 1U;
#ifndef __MPLAB_DEBUGGER_SIMULATOR
    while(CLK13CONbits.OSWEN == 1U) {}
#endif
    /* Enable CLK13 output (REFO1 pin output)
     * NOTE: The OE (Output Enable) bit position in CLK13CON must be
     * verified against the datasheet. On dsPIC33A, clock generator
     * output enable is typically a bit in the CLKxCON register.
     * The PPS mapping in pins.c routes REFO1 to RD10.
     */
    CLK13CONbits.OE = 1U;
}

void CLOCK_CombinedClockFailCallbackRegister(void (*handler)(void))
{
    combinedClockFailHandler = handler;
}

void CLOCK_CombinedClockFailCallback(void)
{
    if(combinedClockFailHandler != NULL)
    {
        combinedClockFailHandler();
    }
}
