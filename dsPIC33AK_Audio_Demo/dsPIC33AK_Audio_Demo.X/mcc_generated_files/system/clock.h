/**
 * CLOCK Generated Driver Header File
 * 
 * @file      clock.h
 * @defgroup  clockdriver Clock Driver
 * @brief     Clock configuration for dsPIC33AK512MPS512 Audio Demo
 * @version   PLIB Version 1.1.2
 * @skipline  Device : dsPIC33AK512MPS512
 *
 * Clock architecture for audio processing:
 *   - System Clock (CLK GEN 1): 200 MHz from PLL2
 *   - SPI/I2S Clock (CLK GEN 6): 320 MHz from PLL1 (for audio bit clock generation)
 *   - ADC Clock (CLK GEN 6): 320 MHz from PLL1
 *   - CCP/REFCLK (CLK GEN 13): Configured for 12.288 MHz MCLK to codec
 *   - Standard Peripheral Clock: 100 MHz (Fosc/2)
 */

#ifndef CLOCK_H
#define CLOCK_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "clock_types.h"

/* ---- Clock frequency macros ---- */
#define CLOCK_SystemFrequencyGet()              (200000000UL)   /* 200 MHz */
#define CLOCK_InstructionFrequencyGet()         ((uint32_t)CLOCK_SystemFrequencyGet())
#define CLOCK_FastPeripheralFrequencyGet()      ((uint32_t)CLOCK_SystemFrequencyGet())
#define CLOCK_StandardPeripheralFrequencyGet()  ((uint32_t)CLOCK_SystemFrequencyGet() / 2U)
#define CLOCK_SlowPeripheralFrequencyGet()      ((uint32_t)CLOCK_SystemFrequencyGet() / 4U)

/* Audio-specific clock definitions */
#define AUDIO_MCLK_FREQUENCY        (12288000UL)    /* 12.288 MHz Master Clock */
#define AUDIO_SAMPLE_RATE           (48000U)         /* 48 kHz sample rate */
#define AUDIO_BIT_DEPTH             (16U)            /* 16-bit samples */
#define AUDIO_CHANNELS              (2U)             /* Stereo */
#define AUDIO_BCLK_FREQUENCY        (AUDIO_SAMPLE_RATE * AUDIO_BIT_DEPTH * AUDIO_CHANNELS)

/**
 * @brief Initialize all clock generators, PLLs, and oscillator sources
 */
void CLOCK_Initialize(void);

/**
 * @brief Get frequency of a specific clock generator
 * @param generator Clock generator enum value
 * @return Frequency in Hz
 */
inline static uint32_t CLOCK_GeneratorFrequencyGet(enum CLOCK_GENERATOR generator)
{
    uint32_t genFrequency = 0x0U;
    switch(generator)
    {
        case CLOCK_GENERATOR_1:  genFrequency = 200000000UL; break;  /* System */
        case CLOCK_GENERATOR_2:  genFrequency = 8000000UL;   break;  /* FRC */
        case CLOCK_GENERATOR_3:  genFrequency = 8000000UL;   break;  /* BFRC/WDT */
        case CLOCK_GENERATOR_6:  genFrequency = 320000000UL; break;  /* ADC/Fast */
        case CLOCK_GENERATOR_10: genFrequency = 80000000UL;  break;  /* CAN */
        case CLOCK_GENERATOR_13: genFrequency = 8000000UL;   break;  /* CCP/REFO */
        default: break;
    }
    return genFrequency;
}

/**
 * @brief Get and clear clock fail status
 */
inline static uint32_t CLOCK_FailStatusGet(void)
{
    uint32_t failStatus = (uint32_t)CLKFAIL;
    CLKFAIL = 0x0U;
    return failStatus;
}

void CLOCK_CombinedClockFailCallbackRegister(void (*handler)(void));
void CLOCK_CombinedClockFailCallback(void);

#endif /* CLOCK_H */
