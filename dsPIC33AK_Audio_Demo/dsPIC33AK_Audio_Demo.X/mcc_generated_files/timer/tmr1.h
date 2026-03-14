/**
 * TMR1 Generated Driver Header File
 * 
 * @file      tmr1.h
 * @ingroup   timerdriver
 * @brief     TMR1 configured as 1 ms system tick
 * @version   PLIB Version 1.0.1
 * @skipline  Device : dsPIC33AK512MPS512
 */

#ifndef TMR1_H
#define TMR1_H

#include <stddef.h>
#include <stdint.h>
#include <xc.h>
#include "timer_interface.h"

extern const struct TIMER_INTERFACE Timer1;

#define Timer1_Initialize           TMR1_Initialize
#define Timer1_Deinitialize         TMR1_Deinitialize
#define Timer1_Start                TMR1_Start
#define Timer1_Stop                 TMR1_Stop
#define Timer1_PeriodSet            TMR1_PeriodSet
#define Timer1_PeriodGet            TMR1_PeriodGet
#define Timer1_CounterGet           TMR1_CounterGet
#define Timer1_Counter16BitGet      TMR1_Counter16BitGet
#define Timer1_InterruptPrioritySet TMR1_InterruptPrioritySet
#define Timer1_TimeoutCallbackRegister TMR1_TimeoutCallbackRegister

void TMR1_Initialize(void);
void TMR1_Deinitialize(void);
void TMR1_Start(void);
void TMR1_Stop(void);
void TMR1_PeriodSet(uint32_t count);

inline static uint32_t TMR1_PeriodGet(void) { return PR1; }
inline static uint32_t TMR1_CounterGet(void) { return TMR1; }
inline static uint16_t TMR1_Counter16BitGet(void) { return (uint16_t)TMR1; }

void TMR1_InterruptPrioritySet(enum INTERRUPT_PRIORITY priority);
void TMR1_TimeoutCallbackRegister(void (*handler)(void));
void TMR1_TimeoutCallback(void);

#endif /* TMR1_H */
