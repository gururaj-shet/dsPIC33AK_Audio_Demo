/**
 * TIMER Interface Header File
 * 
 * @file      timer_interface.h
 * @defgroup  timerdriver Timer Driver
 * @brief     Timer driver interface structure
 * @skipline  Device : dsPIC33AK512MPS512
 */

#ifndef TIMER_INTERFACE_H
#define TIMER_INTERFACE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "../system/interrupt_types.h"

struct TIMER_INTERFACE
{
    void (*Initialize)(void);
    void (*Deinitialize)(void);
    void (*Start)(void);
    void (*Stop)(void);
    void (*PeriodSet)(uint32_t count);
    uint32_t (*PeriodGet)(void);
    uint32_t (*CounterGet)(void);
    void (*InterruptPrioritySet)(enum INTERRUPT_PRIORITY priority);
    void (*TimeoutCallbackRegister)(void (*CallbackHandler)(void));
    void (*Tasks)(void);
};

#endif /* TIMER_INTERFACE_H */
