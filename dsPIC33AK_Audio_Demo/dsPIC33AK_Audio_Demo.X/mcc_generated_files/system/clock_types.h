/**
 * CLOCK Types Header File
 * 
 * @file      clock_types.h
 * @defgroup  clockdriver Clock Driver
 * @brief     Clock type definitions for dsPIC33AK512MPS512
 * @version   PLIB Version 1.1.2
 * @skipline  Device : dsPIC33AK512MPS512
 */

#ifndef CLOCK_TYPES_H
#define CLOCK_TYPES_H

/**
 * @brief Clock generator enumeration for dsPIC33A
 * 
 * dsPIC33A has non-contiguous clock generators:
 *   GEN1  = System clock (200 MHz)
 *   GEN2  = FRC (8 MHz)
 *   GEN3  = BFRC/WDT (8 MHz)
 *   GEN6  = ADC fast clock (320 MHz)
 *   GEN10 = CAN clock (80 MHz)
 *   GEN13 = CCP/REFO1 (8 MHz base, configurable)
 */
enum CLOCK_GENERATOR
{
    CLOCK_GENERATOR_1  = 1,
    CLOCK_GENERATOR_2  = 2,
    CLOCK_GENERATOR_3  = 3,
    CLOCK_GENERATOR_6  = 6,
    CLOCK_GENERATOR_10 = 10,
    CLOCK_GENERATOR_13 = 13,

    /* Semantic aliases */
    CLOCK_SYSTEM = 1,
    CLOCK_FRC    = 2,
    CLOCK_WDT    = 3,
    CLOCK_BFRC   = 3,
    CLOCK_ADC    = 6,
    CLOCK_CAN    = 10,
    CLOCK_CCP    = 13,
    CLOCK_REFO1  = 13,

    CLOCK_GENERATOR_MAX = 6
};

enum CLOCK_FAIL_STATUS_MASKS
{
    CLOCK_GEN1_FAIL_MASK   = 0x1,
    CLOCK_GEN2_FAIL_MASK   = 0x2,
    CLOCK_GEN3_FAIL_MASK   = 0x4,
    CLOCK_GEN6_FAIL_MASK   = 0x20,
    CLOCK_GEN10_FAIL_MASK  = 0x200,
    CLOCK_GEN13_FAIL_MASK  = 0x1000,
    CLOCK_PLL1_FAIL_MASK   = 0x1000000,
    CLOCK_PLL2_FAIL_MASK   = 0x2000000,
};

enum CLOCK_MONITOR
{
    CLOCK_MONITOR_MAX = 0
};

#endif /* CLOCK_TYPES_H */
