/**
 * UART Interface Header File
 * 
 * @file      uart_interface.h
 * @defgroup  uartdriver UART Driver
 * @brief     UART driver interface structure
 * @version   PLIB Version 1.0.2
 * @skipline  Device : dsPIC33AK512MPS512
 */

#ifndef UART_INTERFACE_H
#define UART_INTERFACE_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "uart_types.h"

/**
 * @brief Structure containing function pointers for UART driver abstraction
 */
struct UART_INTERFACE{
    void (*Initialize)(void);
    void (*Deinitialize)(void);
    uint8_t (*Read)(void);
    void (*Write)(uint8_t);
    bool (*IsRxReady)(void);
    bool (*IsTxReady)(void);
    bool (*IsTxDone)(void);
    void (*TransmitEnable)(void);
    void (*TransmitDisable)(void);
    void (*TransmitInterruptEnable)(void);
    void (*TransmitInterruptDisable)(void);
    void (*AutoBaudSet)(bool enable);
    bool (*AutoBaudQuery)(void);
    bool (*AutoBaudEventEnableGet)(void);
    void (*BRGCountSet)(uint32_t brgValue);
    uint32_t (*BRGCountGet)(void);
    void (*BaudRateSet)(uint32_t baudRate);
    uint32_t (*BaudRateGet)(void);
    size_t (*ErrorGet)(void);
    void (*RxCompleteCallbackRegister)(void (*CallbackHandler)(void));
    void (*TxCompleteCallbackRegister)(void (*CallbackHandler)(void));
    void (*TxCollisionCallbackRegister)(void (*CallbackHandler)(void));
    void (*FramingErrorCallbackRegister)(void (*CallbackHandler)(void));
    void (*OverrunErrorCallbackRegister)(void (*CallbackHandler)(void));
    void (*ParityErrorCallbackRegister)(void (*CallbackHandler)(void));
};

#endif /* UART_INTERFACE_H */
