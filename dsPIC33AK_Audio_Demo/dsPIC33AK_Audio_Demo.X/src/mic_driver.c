/**
 * MIC 2 Click Driver
 * 
 * @file      mic_driver.c
 * @brief     Driver for MikroElektronika MIC 2 Click board
 *
 * The MIC 2 Click provides:
 *   - Electret condenser microphone with preamp
 *   - Digital potentiometer (I2C, addr 0x2C) for gain control
 *   - Analog audio output on AN pin (to MCU ADC)
 *
 * Connection to dsPIC33AK Curiosity Board:
 *   - AN pin -> RA0 (mikroBUS socket 1 AN)
 *   - SCL    -> RD7 (shared I2C1 bus with codec)
 *   - SDA    -> RD8 (shared I2C1 bus with codec)
 *   - VCC    -> 3.3V
 *   - GND    -> GND
 */

#include "../include/mic_driver.h"
#include "../mcc_generated_files/i2c/i2c1.h"
#include "../mcc_generated_files/adc/adc5.h"

/* MIC 2 Click digital potentiometer I2C address */
#define MIC2_POT_ADDR       0x2C

/* MCP4561 digital pot command bytes */
#define MIC2_POT_WIPER_CMD  0x00    /* Write to wiper register */

/* Default gain (mid-range) */
static uint8_t currentGain = 128;

void MIC2_Initialize(void)
{
    /* Set default gain to mid-range */
    MIC2_SetGain(128);
}

void MIC2_SetGain(uint8_t gain)
{
    /* Write gain value to digital potentiometer
     * Command format for MCP4561:
     *   Byte 0: [CMD(4)] [ADDR(4)] = 0x00 (write wiper 0)
     *   Byte 1: [DATA(8)]
     */
    currentGain = gain;
    I2C1_WriteRegister(MIC2_POT_ADDR, MIC2_POT_WIPER_CMD, gain);
}

uint8_t MIC2_GetGain(void)
{
    return currentGain;
}

int16_t MIC2_ReadSample(void)
{
    /* Trigger ADC5 Channel 0 conversion and read result */
    ADC5_Channel0SoftwareTrigger();
    while(!ADC5_Channel0IsReady()) {}
    return ADC5_Channel0ResultGet();
}
