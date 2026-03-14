/**
 * INTERRUPT Types Header File
 * 
 * @file      interrupt_types.h
 * @defgroup  interruptdriver Interrupt Driver
 * @brief     Interrupt priority type definitions for dsPIC33AK512MPS512
 * @version   PLIB Version 1.1.0
 * @skipline  Device : dsPIC33AK512MPS512
 */

#ifndef INTERRUPT_TYPES_H
#define INTERRUPT_TYPES_H

/**
 * @brief Interrupt priority levels (0 = disabled, 7 = highest)
 */
enum INTERRUPT_PRIORITY
{
    INTERRUPT_PRIORITY_DISABLED = 0,
    INTERRUPT_PRIORITY_1 = 1,
    INTERRUPT_PRIORITY_2 = 2,
    INTERRUPT_PRIORITY_3 = 3,
    INTERRUPT_PRIORITY_4 = 4,
    INTERRUPT_PRIORITY_5 = 5,
    INTERRUPT_PRIORITY_6 = 6,
    INTERRUPT_PRIORITY_7 = 7,
};

#endif /* INTERRUPT_TYPES_H */
