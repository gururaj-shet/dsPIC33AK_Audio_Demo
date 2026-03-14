/**
 * SYSTEM Generated Driver Header File
 * 
 * @file      system.h
 * @defgroup  systemdriver System Driver
 * @brief     Top-level system initialization for dsPIC33AK512MPS512 Audio Demo
 * @skipline  Device : dsPIC33AK512MPS512
 */

#ifndef SYSTEM_H
#define SYSTEM_H

#include "xc.h"
#include "stdint.h"

/**
 * @brief Initialize all system peripherals
 * 
 * Calls initialization functions for:
 *   - Clock (PLLs, clock generators)
 *   - Pins (GPIO, PPS)
 *   - SPI1 (I2S audio data interface)
 *   - I2C1 (Codec control, MIC 2 Click gain)
 *   - DMA (Audio buffer transfers)
 *   - TMR1 (1 ms system tick)
 *   - UART1 (Debug console / CLI)
 *   - ADC (MIC 2 Click analog input)
 *   - Interrupts
 */
void SYSTEM_Initialize(void);

/**
 * @brief Get base address of device ID register
 */
inline static uint32_t SYSTEM_DeviceIdRegisterAddressGet(void)
{
    return __DEVID_BASE;
}

#endif /* SYSTEM_H */
