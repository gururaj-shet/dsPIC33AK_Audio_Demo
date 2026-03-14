/**
 * SYSTEM Generated Driver Source File
 * 
 * @file      system.c
 * @ingroup   systemdriver
 * @brief     Master system initialization for dsPIC33AK512MPS512 Audio Demo
 * @skipline  Device : dsPIC33AK512MPS512
 *
 * Initialization order matters:
 *   1. Clock (must be first - all peripherals depend on clock)
 *   2. Pins (GPIO directions and PPS before peripherals use them)
 *   3. SPI1 (I2S audio interface)
 *   4. I2C1 (Codec control bus)
 *   5. DMA (Audio buffer transfers)
 *   6. TMR1 (System tick timer)
 *   7. UART1 (Debug console)
 *   8. ADC5 (Microphone analog input via multi-core ADC)
 *   9. Interrupts (enable last, after all peripherals configured)
 */

#include "../system.h"
#include "../clock.h"
#include "../pins.h"
#include "../../spi/spi1.h"
#include "../../i2c/i2c1.h"
#include "../../dma/dma.h"
#include "../../timer/tmr1.h"
#include "../../uart/uart1.h"
#include "../../adc/adc5.h"
#include "../interrupt.h"

void SYSTEM_Initialize(void)
{
    CLOCK_Initialize();
    PINS_Initialize();
    SPI1_Initialize();
    I2C1_Initialize();
    DMA_Initialize();
    TMR1_Initialize();
    UART1_Initialize();
    ADC5_Initialize();
    INTERRUPT_GlobalEnable();
    INTERRUPT_Initialize();
}
