/**
 * UART1 Generated Driver Header File
 * 
 * @file      uart1.h
 * @ingroup   uartdriver
 * @brief     UART1 driver for debug console and CLI
 * @version   PLIB Version 1.0.2
 * @skipline  Device : dsPIC33AK512MPS512
 */

#ifndef UART1_H
#define UART1_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "uart_interface.h"

extern const struct UART_INTERFACE UART1_Drv;

void UART1_Initialize(void);
void UART1_Deinitialize(void);
uint8_t UART1_Read(void);
void UART1_Write(uint8_t data);
bool UART1_IsRxReady(void);
bool UART1_IsTxReady(void);
bool UART1_IsTxDone(void);
void UART1_TransmitEnable(void);
void UART1_TransmitDisable(void);
void UART1_AutoBaudSet(bool enable);
bool UART1_AutoBaudQuery(void);
bool UART1_AutoBaudEventEnableGet(void);
void UART1_BRGCountSet(uint32_t brgValue);
uint32_t UART1_BRGCountGet(void);
void UART1_BaudRateSet(uint32_t baudRate);
uint32_t UART1_BaudRateGet(void);
size_t UART1_ErrorGet(void);

#endif /* UART1_H */
