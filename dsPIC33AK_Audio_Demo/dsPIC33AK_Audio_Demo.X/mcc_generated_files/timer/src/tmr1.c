/**
 * TMR1 Generated Driver Source File
 * 
 * @file      tmr1.c
 * @ingroup   timerdriver
 * @brief     TMR1 as 1 ms system tick for dsPIC33AK512MPS512
 * @version   PLIB Version 1.0.1
 * @skipline  Device : dsPIC33AK512MPS512
 *
 * Timer1 Configuration:
 *   - Clock: Standard Speed Peripheral Clock (100 MHz)
 *   - Period: 1 ms = 100,000 counts at 100 MHz -> PR1 = 99999
 *   - Prescaler: 1:1
 *   - Interrupt on period match
 */

#include "../tmr1.h"
#include "../timer_interface.h"

static void (*TMR1_TimeoutHandler)(void) = NULL;

const struct TIMER_INTERFACE Timer1 = {
    .Initialize            = &TMR1_Initialize,
    .Deinitialize          = &TMR1_Deinitialize,
    .Start                 = &TMR1_Start,
    .Stop                  = &TMR1_Stop,
    .PeriodSet             = &TMR1_PeriodSet,
    .PeriodGet             = &TMR1_PeriodGet,
    .CounterGet            = &TMR1_CounterGet,
    .InterruptPrioritySet  = &TMR1_InterruptPrioritySet,
    .TimeoutCallbackRegister = &TMR1_TimeoutCallbackRegister,
    .Tasks                 = NULL
};

void TMR1_Initialize(void)
{
    T1CON = 0x0UL;              /* TCS=Fperi, 1:1 prescale, disabled */
    TMR1 = 0x0UL;
    PR1 = 0x1869FUL;            /* 99999 -> 1 ms at 100 MHz */

    TMR1_TimeoutCallbackRegister(&TMR1_TimeoutCallback);

    IFS1bits.T1IF = 0;          /* Clear interrupt flag */
    IEC1bits.T1IE = 1;          /* Enable interrupt */
}

void TMR1_Deinitialize(void)
{
    TMR1_Stop();
    IEC1bits.T1IE = 0;
    T1CON = 0x0UL;
    TMR1 = 0x0UL;
    PR1 = 0xFFFFFFFFUL;
}

void TMR1_Start(void)  { T1CONbits.ON = 1; }
void TMR1_Stop(void)   { T1CONbits.ON = 0; }

void TMR1_PeriodSet(uint32_t count) { PR1 = count; }

void TMR1_InterruptPrioritySet(enum INTERRUPT_PRIORITY priority)
{
    IPC6bits.T1IP = priority;
}

void TMR1_TimeoutCallbackRegister(void (*handler)(void))
{
    if(handler != NULL) TMR1_TimeoutHandler = handler;
}

void __attribute__((weak)) TMR1_TimeoutCallback(void) { }

void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void)
{
    if(TMR1_TimeoutHandler != NULL) TMR1_TimeoutHandler();
    IFS1bits.T1IF = 0;
}
