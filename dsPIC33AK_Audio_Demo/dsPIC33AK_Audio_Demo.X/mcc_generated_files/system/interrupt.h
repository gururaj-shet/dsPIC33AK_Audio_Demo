/**
 * INTERRUPT Generated Driver Header File
 * 
 * @file      interrupt.h
 * @defgroup  interruptdriver Interrupt Driver
 * @brief     Interrupt priority configuration for dsPIC33AK512MPS512 Audio Demo
 * @version   PLIB Version 1.1.0
 * @skipline  Device : dsPIC33AK512MPS512
 */

#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>
#include <xc.h>
#include "interrupt_types.h"

/**
 * @brief Initialize interrupt priorities for all audio peripherals
 */
void INTERRUPT_Initialize(void);

/**
 * @brief Deinitialize interrupt priorities to POR defaults
 */
void INTERRUPT_Deinitialize(void);

/**
 * @brief Enable global interrupts
 */
inline static void INTERRUPT_GlobalEnable(void)
{
    __builtin_enable_interrupts();
}

/**
 * @brief Disable global interrupts
 */
inline static void INTERRUPT_GlobalDisable(void)
{
    __builtin_disable_interrupts();
}

/**
 * @brief Get current interrupt vector number
 */
inline static uint16_t INTERRUPT_VectorNumberGet(void)
{
    return _VECNUM;
}

#endif /* INTERRUPT_H */
