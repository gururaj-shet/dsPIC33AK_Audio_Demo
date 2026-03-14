/**
 * PINS Generated Driver Source File
 * 
 * @file      pins.c
 * @ingroup   pinsdriver
 * @brief     Pin initialization for dsPIC33AK512MPS512 Audio Demo
 * @version   PLIB Version 1.0.1
 * @skipline  Device : dsPIC33AK512MPS512
 *
 * Pin mapping for audio peripherals:
 * 
 * SPI1 (I2S mode) - Audio data:
 *   SCK1 -> RD3   (Bit Clock / BCLK)
 *   SDO1 -> RD4   (Data Out to Codec DAC)
 *   SDI1 -> RD5   (Data In from Codec ADC)
 *   SS1  -> RD6   (Word Select / LRCK)
 *
 * I2C1 - Codec control + MIC 2 Click:
 *   SCL1 -> RD7
 *   SDA1 -> RD8
 *
 * UART1 - Debug/CLI:
 *   U1RX -> RD1   (from USB-CDC on Curiosity board)
 *   U1TX -> RH1   (to USB-CDC on Curiosity board)
 *
 * ADC5 - Microphone:
 *   AD5AN0 -> RA0  (MIC 2 Click analog out via ADC5 Channel 0)
 *
 * GPIO:
 *   CODEC_RESET -> RE3  (output, active low)
 *   CODEC_PDN   -> RE4  (output, active low)
 *   LED_STATUS  -> RC8  (output)
 *   LED_CLIP    -> RC9  (output)
 *   BUTTON_S1   -> RC10 (input)
 *   BUTTON_S2   -> RC11 (input)
 *
 * REFCLK:
 *   REFO1 -> RD10 (12.288 MHz MCLK output to codec)
 *
 * Ported from dsPIC33F/E: Replaced _RPnR and RPINR macros with
 * dsPIC33A RPORxx/RPINRxx register style and added PPS lock/unlock.
 */

#include <xc.h>
#include <stddef.h>
#include "../pins.h"

#define PINS_PPSLock()      (RPCONbits.IOLOCK = 1)
#define PINS_PPSUnlock()    (RPCONbits.IOLOCK = 0)

void PINS_Initialize(void)
{
    /* ---- Output Latch SFRs ---- */
    LATA = 0x0000UL;
    LATB = 0x0000UL;
    LATC = 0x0000UL;
    LATD = 0x0000UL;
    LATE = 0x0018UL;    /* RE3=1, RE4=1 (codec reset/pdn deasserted initially) */
    LATF = 0x0000UL;
    LATG = 0x0000UL;
    LATH = 0x0002UL;    /* RH1 TX idle high */

    /* ---- GPIO Direction ---- */
    TRISA = 0xFFFFUL;   /* RA0 = analog input for MIC 2 */
    TRISB = 0xFFFFUL;
    TRISC = 0x0CFEUL;   /* RC8, RC9 = output (LEDs); RC10, RC11 = input (buttons) */
    TRISD = 0xFDE2UL;   /* RD3,RD4,RD6,RD10 = output; RD1,RD5,RD7,RD8 = input/OD */
    TRISE = 0x07E7UL;   /* RE3, RE4 = output (codec ctrl) */
    TRISF = 0x0FEFUL;
    TRISG = 0x03F7UL;
    TRISH = 0x0005UL;   /* RH1 = output (U1TX) */

    /* ---- Weak Pull-Ups for buttons ---- */
    CNPUA = 0x0000UL;
    CNPUB = 0x0000UL;
    CNPUC = 0x0C00UL;   /* Pull-up on RC10 (S1) and RC11 (S2) */
    CNPUD = 0x0000UL;
    CNPUE = 0x0000UL;
    CNPUF = 0x0000UL;
    CNPUG = 0x0000UL;
    CNPUH = 0x0000UL;

    /* ---- Pull-Downs ---- */
    CNPDA = 0x0000UL;
    CNPDB = 0x0000UL;
    CNPDC = 0x0000UL;
    CNPDD = 0x0000UL;
    CNPDE = 0x0000UL;
    CNPDF = 0x0000UL;
    CNPDG = 0x0000UL;
    CNPDH = 0x0000UL;

    /* ---- Open Drain ---- */
    ODCA = 0x0000UL;
    ODCB = 0x0000UL;
    ODCC = 0x0000UL;
    ODCD = 0x0180UL;    /* RD7, RD8 open-drain for I2C */
    ODCE = 0x0000UL;
    ODCF = 0x0000UL;
    ODCG = 0x0000UL;
    ODCH = 0x0000UL;

    /* ---- Analog/Digital Configuration ---- */
    ANSELA = 0x0001UL;  /* RA0 = analog (MIC 2 Click AN) */
    ANSELB = 0x0000UL;  /* All digital */
    ANSELE = 0x0000UL;
    ANSELF = 0x0000UL;

    /* ---- Peripheral Pin Select (PPS) ---- */
    PINS_PPSUnlock();

    /* UART1 */
    RPINR13bits.U1RXR = 0x0032UL;      /* RD1 -> UART1:U1RX */
    RPOR28bits.RP114R = 0x0013UL;       /* RH1 -> UART1:U1TX */

    /* SPI1 (I2S mode) - Audio interface to codec
     * On dsPIC33A, SPI can operate in I2S/audio mode.
     * SCK1 is auto-mapped when SPI1 is enabled as master on RD3.
     * SDO1, SDI1, and SS1 need PPS mapping.
     *
     * IMPORTANT: PPS output function codes below are estimates.
     * They MUST be verified against the "Output Pin Selection" table
     * in the dsPIC33AK256MPS512 Family Data Sheet (DS70005611) or
     * the DFP device header file (p33AK512MPS512.h).
     *
     * Verified PPS codes from OOB demo:
     *   0x0013 = UART1:U1TX
     *   0x0027 = SCCP1:OCM1
     *   0x0028 = SCCP2:OCM2
     *   0x0029 = SCCP3:OCM3
     *
     * RP number mapping for Port D:
     *   RP49=RD0, RP50=RD1, RP51=RD2, RP52=RD3, RP53=RD4,
     *   RP54=RD5, RP55=RD6, RP56=RD7, RP57=RD8, RP58=RD9,
     *   RP59=RD10  (VERIFY against device pin table)
     *
     * RPORxx grouping: Each RPORxx register holds 2 RP output
     * assignments. E.g., RPOR12 holds RP49R (low) and RP50R (high),
     * or similar. Verify exact register-to-RP mapping.
     */
    RPOR13bits.RP53R = 0x0005UL;        /* RD4 -> SPI1:SDO1 (VERIFY PPS code) */
    RPINR1bits.SDI1R = 54UL;            /* RD5 (RP54) -> SPI1:SDI1 input */
    RPOR13bits.RP55R = 0x0007UL;        /* RD6 -> SPI1:SS1/LRCK (VERIFY PPS code) */

    /* REFCLK output for codec MCLK (12.288 MHz from CLK Generator 13)
     * On dsPIC33A, REFO1 is Clock Generator 13 output, NOT a separate
     * REFOCON peripheral. The PPS code for CLK13/REFO1 output needs
     * verification in the datasheet PPS output function table.
     */
    RPOR14bits.RP59R = 0x000FUL;        /* RD10 -> REFO1/CLK13 out (VERIFY PPS code) */

    PINS_PPSLock();
}
