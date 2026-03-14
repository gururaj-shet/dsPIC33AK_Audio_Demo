/**
 * I2C1 Generated Driver Source File
 * 
 * @file      i2c1.c
 * @ingroup   i2cdriver
 * @brief     I2C1 master driver for dsPIC33AK512MPS512 Audio Demo
 * @version   PLIB Version 1.0.0
 * @skipline  Device : dsPIC33AK512MPS512
 *
 * dsPIC33A I2C register differences from dsPIC33F/E:
 *   - I2CxCON is 32-bit (was 16-bit I2CxCON/I2CxCONL on dsPIC33E)
 *   - I2CxSTAT is 32-bit
 *   - I2CxBRG is 32-bit
 *   - Bit names are largely the same (SEN, PEN, RCEN, ACKDT, etc.)
 *   - Clock source: Standard Peripheral Clock (100 MHz)
 *   - BRG = (Fperi / (2 * Fscl)) - 1
 */

#include <xc.h>
#include <stdint.h>
#include "../i2c1.h"

/* I2C clock calculations */
#define I2C1_CLOCK_FREQ     100000000UL     /* 100 MHz Standard Peripheral Clock */
#define I2C1_BAUD_RATE      400000UL        /* 400 kHz Fast Mode */
#define I2C1_BRG_VALUE      ((I2C1_CLOCK_FREQ / (2UL * I2C1_BAUD_RATE)) - 1UL)

/* Timeout for bus operations (in loop iterations) */
#define I2C1_TIMEOUT        100000UL

/**
 * @brief Wait for an I2C condition to complete with timeout
 */
static bool I2C1_WaitForIdle(void)
{
    uint32_t timeout = I2C1_TIMEOUT;
    while((I2C1CONbits.SEN || I2C1CONbits.PEN || I2C1CONbits.RCEN ||
           I2C1CONbits.RSEN || I2C1CONbits.ACKEN || I2C1STATbits.TRSTAT) &&
          (--timeout > 0))
    {
        /* Wait */
    }
    return (timeout > 0);
}

void I2C1_Initialize(void)
{
    /* Disable I2C1 before configuration */
    I2C1CONbits.ON = 0;

    /* Configure I2C1 as Master, 400 kHz */
    I2C1CON = 0x00000000UL;
    I2C1BRG = I2C1_BRG_VALUE;
    I2C1STAT = 0x00000000UL;

    /* Enable I2C1 */
    I2C1CONbits.ON = 1;
}

void I2C1_Deinitialize(void)
{
    I2C1CONbits.ON = 0;
    I2C1CON = 0x00000000UL;
}

bool I2C1_WriteRegister(uint8_t slaveAddr, uint8_t regAddr, uint8_t data)
{
    if(!I2C1_WaitForIdle()) return false;

    /* Start condition */
    I2C1CONbits.SEN = 1;
    if(!I2C1_WaitForIdle()) return false;

    /* Send slave address with write bit */
    I2C1TRN = (uint32_t)(slaveAddr << 1) | 0x00;
    if(!I2C1_WaitForIdle()) return false;
    if(I2C1STATbits.ACKSTAT) return false;  /* NACK received */

    /* Send register address */
    I2C1TRN = (uint32_t)regAddr;
    if(!I2C1_WaitForIdle()) return false;
    if(I2C1STATbits.ACKSTAT) return false;

    /* Send data byte */
    I2C1TRN = (uint32_t)data;
    if(!I2C1_WaitForIdle()) return false;
    if(I2C1STATbits.ACKSTAT) return false;

    /* Stop condition */
    I2C1CONbits.PEN = 1;
    if(!I2C1_WaitForIdle()) return false;

    return true;
}

bool I2C1_ReadRegister(uint8_t slaveAddr, uint8_t regAddr, uint8_t *data)
{
    if(!I2C1_WaitForIdle()) return false;

    /* Start condition */
    I2C1CONbits.SEN = 1;
    if(!I2C1_WaitForIdle()) return false;

    /* Send slave address with write bit (to set register pointer) */
    I2C1TRN = (uint32_t)(slaveAddr << 1) | 0x00;
    if(!I2C1_WaitForIdle()) return false;
    if(I2C1STATbits.ACKSTAT) return false;

    /* Send register address */
    I2C1TRN = (uint32_t)regAddr;
    if(!I2C1_WaitForIdle()) return false;
    if(I2C1STATbits.ACKSTAT) return false;

    /* Repeated Start */
    I2C1CONbits.RSEN = 1;
    if(!I2C1_WaitForIdle()) return false;

    /* Send slave address with read bit */
    I2C1TRN = (uint32_t)(slaveAddr << 1) | 0x01;
    if(!I2C1_WaitForIdle()) return false;
    if(I2C1STATbits.ACKSTAT) return false;

    /* Enable receive */
    I2C1CONbits.RCEN = 1;
    if(!I2C1_WaitForIdle()) return false;

    /* Read data */
    *data = (uint8_t)(I2C1RCV & 0xFF);

    /* Send NACK (last byte) */
    I2C1CONbits.ACKDT = 1;     /* NACK */
    I2C1CONbits.ACKEN = 1;
    if(!I2C1_WaitForIdle()) return false;

    /* Stop condition */
    I2C1CONbits.PEN = 1;
    if(!I2C1_WaitForIdle()) return false;

    return true;
}

bool I2C1_WriteRegisters(uint8_t slaveAddr, uint8_t regAddr,
                         const uint8_t *data, uint8_t length)
{
    uint8_t i;

    if(!I2C1_WaitForIdle()) return false;

    /* Start */
    I2C1CONbits.SEN = 1;
    if(!I2C1_WaitForIdle()) return false;

    /* Slave address + write */
    I2C1TRN = (uint32_t)(slaveAddr << 1) | 0x00;
    if(!I2C1_WaitForIdle()) return false;
    if(I2C1STATbits.ACKSTAT) return false;

    /* Register address */
    I2C1TRN = (uint32_t)regAddr;
    if(!I2C1_WaitForIdle()) return false;
    if(I2C1STATbits.ACKSTAT) return false;

    /* Data bytes */
    for(i = 0; i < length; i++)
    {
        I2C1TRN = (uint32_t)data[i];
        if(!I2C1_WaitForIdle()) return false;
        if(I2C1STATbits.ACKSTAT) return false;
    }

    /* Stop */
    I2C1CONbits.PEN = 1;
    if(!I2C1_WaitForIdle()) return false;

    return true;
}
