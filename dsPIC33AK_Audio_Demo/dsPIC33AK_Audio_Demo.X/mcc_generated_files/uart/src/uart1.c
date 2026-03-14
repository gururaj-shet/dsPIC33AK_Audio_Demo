/**
 * UART1 Generated Driver Source File
 * 
 * @file      uart1.c
 * @ingroup   uartdriver
 * @brief     UART1 at 115200 baud for debug/CLI on dsPIC33AK512MPS512
 * @version   PLIB Version 1.0.2
 * @skipline  Device : dsPIC33AK512MPS512
 *
 * dsPIC33A UART register differences from dsPIC33F/E:
 *   - UxCON is 32-bit (replaces UxMODE + UxSTA)
 *   - UxSTAT is 32-bit status register
 *   - UxBRG is 32-bit
 *   - Fractional baud rate mode via CLKMOD bit
 *   - TX/RX buffers are UxTXB / UxRXB
 */

#include <xc.h>
#include <stdint.h>
#include <stddef.h>
#include "../uart1.h"

#define UART1_CLOCK 100000000U
#define UART1_BAUD_TO_BRG_WITH_FRACTIONAL(x)  (UART1_CLOCK/(x))
#define UART1_BAUD_TO_BRG_WITH_BRGS_1(x)      (UART1_CLOCK/(4U*(x))-1U)
#define UART1_BAUD_TO_BRG_WITH_BRGS_0(x)      (UART1_CLOCK/(16U*(x))-1U)
#define UART1_BRG_TO_BAUD_WITH_FRACTIONAL(x)  (UART1_CLOCK/(x))
#define UART1_BRG_TO_BAUD_WITH_BRGS_1(x)      (UART1_CLOCK/(4U*((x)+1U)))
#define UART1_BRG_TO_BAUD_WITH_BRGS_0(x)      (UART1_CLOCK/(16U*((x)+1U)))

#define UART1_MIN_ACHIEVABLE_BAUD_WITH_FRACTIONAL  95U
#define UART1_MIN_ACHIEVABLE_BAUD_WITH_BRGS_1      24U

/* Driver interface vtable */
const struct UART_INTERFACE UART1_Drv = {
    .Initialize = &UART1_Initialize,
    .Deinitialize = &UART1_Deinitialize,
    .Read = &UART1_Read,
    .Write = &UART1_Write,
    .IsRxReady = &UART1_IsRxReady,
    .IsTxReady = &UART1_IsTxReady,
    .IsTxDone = &UART1_IsTxDone,
    .TransmitEnable = &UART1_TransmitEnable,
    .TransmitDisable = &UART1_TransmitDisable,
    .TransmitInterruptEnable = NULL,
    .TransmitInterruptDisable = NULL,
    .AutoBaudSet = &UART1_AutoBaudSet,
    .AutoBaudQuery = &UART1_AutoBaudQuery,
    .AutoBaudEventEnableGet = &UART1_AutoBaudEventEnableGet,
    .BRGCountSet = &UART1_BRGCountSet,
    .BRGCountGet = &UART1_BRGCountGet,
    .BaudRateSet = &UART1_BaudRateSet,
    .BaudRateGet = &UART1_BaudRateGet,
    .ErrorGet = &UART1_ErrorGet,
    .RxCompleteCallbackRegister = NULL,
    .TxCompleteCallbackRegister = NULL,
    .TxCollisionCallbackRegister = NULL,
    .FramingErrorCallbackRegister = NULL,
    .OverrunErrorCallbackRegister = NULL,
    .ParityErrorCallbackRegister = NULL,
};

static union {
    struct {
        uint16_t frammingError     :1;
        uint16_t parityError       :1;
        uint16_t overrunError      :1;
        uint16_t txCollisionError  :1;
        uint16_t autoBaudOverflow  :1;
        uint16_t reserved          :11;
    };
    size_t status;
} uartError;

void UART1_Initialize(void)
{
    /* Asynchronous 8-bit UART, 1 stop bit, no parity */
    U1CON = 0x8000000UL;
    U1STAT = 0x2E0080UL;
    /* BaudRate ~115207 bps; Frequency 100 MHz; BRG 868 */
    U1BRG = 0x364UL;

    U1CONbits.ON = 1;
    U1CONbits.TXEN = 1;
    U1CONbits.RXEN = 1;
}

void UART1_Deinitialize(void)
{
    U1CON = 0x0UL;
    U1STAT = 0x2E0080UL;
    U1BRG = 0x0UL;
}

uint8_t UART1_Read(void)
{
    while(U1STATbits.RXBE == 1) {}
    if(U1STATbits.RXFOIF == 1) {
        U1STATbits.RXFOIF = 0;
    }
    return U1RXB;
}

void UART1_Write(uint8_t txData)
{
    while(U1STATbits.TXBF == 1) {}
    U1TXB = txData;
}

bool UART1_IsRxReady(void)
{
    return (U1STATbits.RXBE == 0);
}

bool UART1_IsTxReady(void)
{
    return ((!U1STATbits.TXBF) && U1CONbits.TXEN);
}

bool UART1_IsTxDone(void)
{
    return (bool)(U1STATbits.TXMTIF && U1STATbits.TXBE);
}

void UART1_TransmitEnable(void)   { U1CONbits.TXEN = 1; }
void UART1_TransmitDisable(void)  { U1CONbits.TXEN = 0; }

void UART1_AutoBaudSet(bool enable)
{
    U1UIRbits.ABDIF = 0U;
    U1UIRbits.ABDIE = enable;
    U1CONbits.ABDEN = enable;
}

bool UART1_AutoBaudQuery(void)         { return U1CONbits.ABDEN; }
bool UART1_AutoBaudEventEnableGet(void){ return U1UIRbits.ABDIE; }

size_t UART1_ErrorGet(void)
{
    uartError.status = 0;
    if(U1STATbits.FERIF)  uartError.status |= UART_ERROR_FRAMING_MASK;
    if(U1STATbits.PERIF)  uartError.status |= UART_ERROR_PARITY_MASK;
    if(U1STATbits.RXFOIF) { uartError.status |= UART_ERROR_RX_OVERRUN_MASK; U1STATbits.RXFOIF = 0; }
    if(U1STATbits.TXCIF)  { uartError.status |= UART_ERROR_TX_COLLISION_MASK; U1STATbits.TXCIF = 0; }
    if(U1STATbits.ABDOVIF){ uartError.status |= UART_ERROR_AUTOBAUD_OVERFLOW_MASK; U1STATbits.ABDOVIF = 0; }
    return uartError.status;
}

void UART1_BRGCountSet(uint32_t brgValue) { U1BRG = brgValue; }
uint32_t UART1_BRGCountGet(void) { return U1BRG; }

void UART1_BaudRateSet(uint32_t baudRate)
{
    uint32_t brgValue;
    if((baudRate >= UART1_MIN_ACHIEVABLE_BAUD_WITH_FRACTIONAL) && (baudRate != 0U)) {
        U1CONbits.CLKMOD = 1; U1CONbits.BRGS = 0;
        brgValue = UART1_BAUD_TO_BRG_WITH_FRACTIONAL(baudRate);
    } else if(baudRate >= UART1_MIN_ACHIEVABLE_BAUD_WITH_BRGS_1) {
        U1CONbits.CLKMOD = 0; U1CONbits.BRGS = 1;
        brgValue = UART1_BAUD_TO_BRG_WITH_BRGS_1(baudRate);
    } else {
        U1CONbits.CLKMOD = 0; U1CONbits.BRGS = 0;
        brgValue = UART1_BAUD_TO_BRG_WITH_BRGS_0(baudRate);
    }
    U1BRG = brgValue;
}

uint32_t UART1_BaudRateGet(void)
{
    uint32_t brgValue = UART1_BRGCountGet();
    if((U1CONbits.CLKMOD == 1U) && (brgValue != 0U))
        return UART1_BRG_TO_BAUD_WITH_FRACTIONAL(brgValue);
    else if(U1CONbits.BRGS == 1)
        return UART1_BRG_TO_BAUD_WITH_BRGS_1(brgValue);
    else
        return UART1_BRG_TO_BAUD_WITH_BRGS_0(brgValue);
}

/* printf redirect for XC-DSC */
int __attribute__((__section__(".libc.write"))) write(int handle, void *buffer, unsigned int len)
{
    unsigned int i = 0;
    while(!UART1_IsTxDone()) {}
    while(i < len) {
        while(!UART1_IsTxReady()) {}
        UART1_Write(*((uint8_t *)buffer + i++));
    }
    return (int)i;
}
