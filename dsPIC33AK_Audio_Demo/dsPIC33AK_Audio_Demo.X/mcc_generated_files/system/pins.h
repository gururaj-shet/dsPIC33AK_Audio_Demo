/**
 * PINS Generated Driver Header File
 * 
 * @file      pins.h
 * @defgroup  pinsdriver Pins Driver
 * @brief     Pin configuration for dsPIC33AK512MPS512 Audio Demo
 * @version   PLIB Version 1.0.1
 * @skipline  Device : dsPIC33AK512MPS512
 *
 * Pin assignments for Audio Demo:
 * 
 * SPI1 (I2S Audio Data):
 *   SCK1  -> RD3  (BCLK to codec)
 *   SDO1  -> RD4  (Audio data MCU -> Codec DAC)
 *   SDI1  -> RD5  (Audio data Codec ADC -> MCU)
 *   SS1   -> RD6  (LRCK / Word Select)
 *
 * I2C1 (Codec Control + MIC 2 Click Gain):
 *   SCL1  -> RD7
 *   SDA1  -> RD8
 *
 * UART1 (Debug Console / CLI):
 *   U1TX  -> RH1
 *   U1RX  -> RD1
 *
 * ADC5 (MIC 2 Click Analog Input):
 *   AD5AN0 -> RA0 (MIC 2 Click AN pin on mikroBUS socket 1, via ADC5 Ch0)
 *
 * GPIO:
 *   CODEC_RESET -> RE3 (Codec hardware reset, active low)
 *   CODEC_PDN   -> RE4 (Codec power down, active low)
 *   LED_STATUS  -> RC8 (Status LED)
 *   LED_CLIP    -> RC9 (Audio clip indicator LED)
 *   BUTTON_S1   -> RC10 (Effect select button)
 *   BUTTON_S2   -> RC11 (Mode select button)
 *
 * REFCLK Output (MCLK to codec):
 *   REFO1 -> RD10 (12.288 MHz reference clock)
 */

#ifndef PINS_H
#define PINS_H

#include <xc.h>

/* ---- Status LED (RC8) ---- */
#define LED_STATUS_SetHigh()          (_LATC8 = 1)
#define LED_STATUS_SetLow()           (_LATC8 = 0)
#define LED_STATUS_Toggle()           (_LATC8 ^= 1)
#define LED_STATUS_GetValue()         _RC8
#define LED_STATUS_SetDigitalOutput() (_TRISC8 = 0)

/* ---- Clip Indicator LED (RC9) ---- */
#define LED_CLIP_SetHigh()            (_LATC9 = 1)
#define LED_CLIP_SetLow()             (_LATC9 = 0)
#define LED_CLIP_Toggle()             (_LATC9 ^= 1)
#define LED_CLIP_GetValue()           _RC9
#define LED_CLIP_SetDigitalOutput()   (_TRISC9 = 0)

/* ---- Effect Select Button (RC10) ---- */
#define BUTTON_S1_GetValue()          _RC10
#define BUTTON_S1_SetDigitalInput()   (_TRISC10 = 1)

/* ---- Mode Select Button (RC11) ---- */
#define BUTTON_S2_GetValue()          _RC11
#define BUTTON_S2_SetDigitalInput()   (_TRISC11 = 1)

/* ---- Codec Reset (RE3, active low) ---- */
#define CODEC_RESET_SetHigh()         (_LATE3 = 1)
#define CODEC_RESET_SetLow()          (_LATE3 = 0)
#define CODEC_RESET_SetDigitalOutput() (_TRISE3 = 0)

/* ---- Codec Power Down (RE4, active low) ---- */
#define CODEC_PDN_SetHigh()           (_LATE4 = 1)
#define CODEC_PDN_SetLow()            (_LATE4 = 0)
#define CODEC_PDN_SetDigitalOutput()  (_TRISE4 = 0)

/**
 * @brief Initialize all pin directions, PPS mappings, and analog selections
 */
void PINS_Initialize(void);

#endif /* PINS_H */
