/**
 * CONFIGURATION BITS Generated Driver Source File
 * 
 * @file      config_bits.c
 * @defgroup  config_bits_driver Config Bits Driver
 * @brief     Configuration fuse settings for dsPIC33AK512MPS512 Audio Demo
 * @version   PLIB Version 1.1.0
 * @skipline  Device : dsPIC33AK512MPS512
 * 
 * Ported from dsPIC33F/E audio library to dsPIC33A architecture.
 * All fuse settings match the Curiosity Platform Board (EV74H48A)
 * with GP DIM (EV80L65A) hardware.
 */

// FCP - Flash Configuration
#pragma config FCP_CP = OFF             // Code protection disabled
#pragma config FCP_CRC = OFF            // CRC disabled
#pragma config FCP_WPUCA = OFF          // Write protect user config area disabled

// FICD - In-Circuit Debug
#pragma config FICD_JTAGEN = OFF        // JTAG disabled
#pragma config FICD_NOBTSWP = OFF       // Boot swap disabled

// FDEVOPT - Device Options
#pragma config FDEVOPT_ALTI2C1 = OFF    // Alternate I2C1 pins disabled
#pragma config FDEVOPT_ALTI2C2 = OFF    // Alternate I2C2 pins disabled
#pragma config FDEVOPT_ALTI2C3 = OFF    // Alternate I2C3 pins disabled
#pragma config FDEVOPT_BISTDIS = OFF    // BIST disabled
#pragma config FDEVOPT_SPI2PIN = OFF    // SPI2 alternate pins disabled

// FWDT - Watchdog Timer
#pragma config FWDT_WINDIS = ON         // Watchdog windowed mode disabled
#pragma config FWDT_SWDTMPS = PS2147483648
#pragma config FWDT_RCLKSEL = BFRC256   // WDT clock from BFRC/256
#pragma config FWDT_RWDTPS = PS2147483648
#pragma config FWDT_WDTWIN = WIN25
#pragma config FWDT_WDTEN = SW          // WDT controlled by software
#pragma config FWDT_WDTRSTEN = ON       // WDT reset enabled
#pragma config FWDT_WDTNVMSTL = ON

// FIRT - IRT
#pragma config FIRT_IRT = OFF

// FSECDBG
#pragma config FSECDBG_SECDBG = OFF

// FPED
#pragma config FPED_ICSPPED = OFF

// FEPUCB
#pragma config FEPUCB_EPUCB = 0xffffffff

// FWPUCB
#pragma config FWPUCB_WPUCB = 0xffffffff

// FBOOT - Boot Mode
#pragma config FBOOT_BTMODE = SINGLE    // Single boot partition
#pragma config FBOOT_PROG = OFF
