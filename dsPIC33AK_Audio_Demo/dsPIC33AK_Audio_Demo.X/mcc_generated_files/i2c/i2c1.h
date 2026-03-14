/**
 * I2C1 Generated Driver Header File
 * 
 * @file      i2c1.h
 * @ingroup   i2cdriver
 * @brief     I2C1 master driver for codec control and MIC 2 Click gain
 * @version   PLIB Version 1.0.0
 * @skipline  Device : dsPIC33AK512MPS512
 *
 * I2C1 is used for:
 *   - AK4642 codec register configuration (addr 0x12)
 *   - MIC 2 Click digital potentiometer (addr 0x2C)
 *
 * Bus speed: 400 kHz (Fast Mode)
 */

#ifndef I2C1_H
#define I2C1_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

/* I2C slave addresses */
#define I2C1_CODEC_ADDRESS      0x12    /* AK4642 7-bit address */
#define I2C1_MIC2_POT_ADDRESS   0x2C    /* MIC 2 Click digital pot */

/**
 * @brief Initialize I2C1 master at 400 kHz
 */
void I2C1_Initialize(void);

/**
 * @brief Deinitialize I2C1
 */
void I2C1_Deinitialize(void);

/**
 * @brief Write a single byte to a slave device register
 * @param slaveAddr 7-bit slave address
 * @param regAddr Register address
 * @param data Data byte to write
 * @return true on success, false on NACK or error
 */
bool I2C1_WriteRegister(uint8_t slaveAddr, uint8_t regAddr, uint8_t data);

/**
 * @brief Read a single byte from a slave device register
 * @param slaveAddr 7-bit slave address
 * @param regAddr Register address
 * @param data Pointer to store read byte
 * @return true on success, false on NACK or error
 */
bool I2C1_ReadRegister(uint8_t slaveAddr, uint8_t regAddr, uint8_t *data);

/**
 * @brief Write multiple bytes to a slave device
 * @param slaveAddr 7-bit slave address
 * @param regAddr Starting register address
 * @param data Pointer to data buffer
 * @param length Number of bytes to write
 * @return true on success
 */
bool I2C1_WriteRegisters(uint8_t slaveAddr, uint8_t regAddr,
                         const uint8_t *data, uint8_t length);

#endif /* I2C1_H */
