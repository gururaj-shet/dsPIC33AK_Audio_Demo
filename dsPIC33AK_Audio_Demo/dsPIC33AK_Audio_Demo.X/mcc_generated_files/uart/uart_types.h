/**
 * UART Types Header File
 * 
 * @file      uart_types.h
 * @defgroup  uartdriver UART Driver
 * @brief     UART error type definitions
 * @version   PLIB Version 1.0.2
 * @skipline  Device : dsPIC33AK512MPS512
 */

#ifndef UART_TYPES_H
#define UART_TYPES_H

enum UART_ERROR_MASKS{
    UART_ERROR_FRAMING_MASK         = 0x1,
    UART_ERROR_PARITY_MASK          = 0x2,
    UART_ERROR_RX_OVERRUN_MASK      = 0x4,
    UART_ERROR_TX_COLLISION_MASK    = 0x8,
    UART_ERROR_AUTOBAUD_OVERFLOW_MASK = 0x10,
};

#endif /* UART_TYPES_H */
