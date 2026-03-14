/**
 * Audio Codec Driver (AK4642EN on AC328904)
 * 
 * @file      codec_driver.c
 * @brief     AK4642 codec initialization and control via I2C
 * 
 * This driver configures the AK4642EN audio codec on the AC328904
 * Audio Codec Board for stereo audio I/O at 48 kHz / 16-bit.
 *
 * Codec Features Used:
 *   - ADC: Line-in or microphone input
 *   - DAC: Line-out / headphone output
 *   - I2S interface for audio data
 *   - I2C interface for register control
 *
 * Ported from dsPIC33F/E:
 *   - I2C register writes adapted for dsPIC33A I2C1 driver
 *   - Timing delays adjusted for 200 MHz system clock
 *   - No codec register changes needed (codec is device-agnostic)
 */

#include "../include/codec_driver.h"
#include "../mcc_generated_files/i2c/i2c1.h"
#include "../mcc_generated_files/system/pins.h"
#include <stdint.h>
#include <stdbool.h>

/* AK4642 I2C Address (7-bit) */
#define AK4642_I2C_ADDR     0x12

/* ---- AK4642 Register Map (subset for audio demo) ---- */
#define AK4642_REG_PM1          0x00    /* Power Management 1 */
#define AK4642_REG_PM2          0x01    /* Power Management 2 */
#define AK4642_REG_SIG_SEL1     0x02    /* Signal Select 1 */
#define AK4642_REG_SIG_SEL2     0x03    /* Signal Select 2 */
#define AK4642_REG_MODE_CTRL1   0x04    /* Mode Control 1 */
#define AK4642_REG_MODE_CTRL2   0x05    /* Mode Control 2 */
#define AK4642_REG_TMR_SEL      0x06    /* Timer Select */
#define AK4642_REG_ALC_CTRL1    0x07    /* ALC Mode Control 1 */
#define AK4642_REG_ALC_CTRL2    0x08    /* ALC Mode Control 2 */
#define AK4642_REG_L_INPUT_VOL  0x09    /* Lch Input Volume */
#define AK4642_REG_L_DIG_VOL    0x0A    /* Lch Digital Volume */
#define AK4642_REG_ALC_CTRL3    0x0B    /* ALC Mode Control 3 */
#define AK4642_REG_R_INPUT_VOL  0x0D    /* Rch Input Volume */
#define AK4642_REG_R_DIG_VOL    0x0E    /* Rch Digital Volume */
#define AK4642_REG_MODE_CTRL3   0x0F    /* Mode Control 3 */
#define AK4642_REG_MODE_CTRL4   0x10    /* Mode Control 4 */
#define AK4642_REG_PM3          0x11    /* Power Management 3 */
#define AK4642_REG_DIG_FILT_SEL 0x12    /* Digital Filter Select */
#define AK4642_REG_DIG_FILT_MODE 0x13   /* Digital Filter Mode */

/* Static volume tracking */
static uint8_t currentVolume = 0xCC;    /* Default ~80% volume */

/**
 * @brief Simple delay in microseconds (busy-wait)
 * Approximate at 200 MHz instruction clock.
 */
static void DelayUs(uint32_t us)
{
    volatile uint32_t count = us * 50;  /* ~50 cycles per us at 200 MHz */
    while(count--) {}
}

/**
 * @brief Write a value to an AK4642 register
 */
static bool Codec_WriteReg(uint8_t reg, uint8_t value)
{
    return I2C1_WriteRegister(AK4642_I2C_ADDR, reg, value);
}

/**
 * @brief Read a value from an AK4642 register
 */
static bool Codec_ReadReg(uint8_t reg, uint8_t *value)
{
    return I2C1_ReadRegister(AK4642_I2C_ADDR, reg, value);
}

void CODEC_Initialize(void)
{
    /* Hardware reset sequence */
    CODEC_RESET_SetDigitalOutput();
    CODEC_PDN_SetDigitalOutput();

    /* Assert reset (active low) */
    CODEC_RESET_SetLow();
    CODEC_PDN_SetLow();
    DelayUs(10000);     /* Hold reset for 10 ms */

    /* Release reset */
    CODEC_PDN_SetHigh();
    DelayUs(1000);
    CODEC_RESET_SetHigh();
    DelayUs(10000);     /* Wait for codec to stabilize */

    /* ---- Power up sequence ---- */
    /* PM1: PMPFIL=1 (power up programmable filter block) */
    Codec_WriteReg(AK4642_REG_PM1, 0x40);
    DelayUs(1000);

    /* Mode Control 1:
     *   DIF[1:0] = 11 (I2S compatible, 16-bit)
     *   BCKO = 0 (64fs BCLK)
     */
    Codec_WriteReg(AK4642_REG_MODE_CTRL1, 0x03);

    /* Mode Control 2:
     *   FS[3:0] = 0101 (48 kHz sample rate)
     *   CM[1:0] = 00 (256fs master clock = 12.288 MHz)
     */
    Codec_WriteReg(AK4642_REG_MODE_CTRL2, 0x05);

    /* Mode Control 3:
     *   SMUTE = 0 (soft mute disabled)
     *   DEM[1:0] = 00 (no de-emphasis)
     */
    Codec_WriteReg(AK4642_REG_MODE_CTRL3, 0x00);

    /* Signal Select 1:
     *   PMMP = 0 (microphone amp off initially)
     *   MGAIN = 0dB
     *   DAC -> Speaker Amp path
     */
    Codec_WriteReg(AK4642_REG_SIG_SEL1, 0x00);

    /* Signal Select 2:
     *   LOVL = 0 (line out normal level)
     */
    Codec_WriteReg(AK4642_REG_SIG_SEL2, 0x00);

    /* Digital Volume: Set to ~80% */
    Codec_WriteReg(AK4642_REG_L_DIG_VOL, currentVolume);
    Codec_WriteReg(AK4642_REG_R_DIG_VOL, currentVolume);

    /* Input Volume: 0 dB gain */
    Codec_WriteReg(AK4642_REG_L_INPUT_VOL, 0xE1);
    Codec_WriteReg(AK4642_REG_R_INPUT_VOL, 0xE1);

    /* Power Management 2:
     *   PMDAC = 1 (power up DAC)
     *   PMHPL = 1 (power up headphone L)
     *   PMHPR = 1 (power up headphone R)
     */
    Codec_WriteReg(AK4642_REG_PM2, 0x07);
    DelayUs(30000);     /* Wait for DAC power up (30 ms) */

    /* Power Management 1:
     *   PMADL = 1 (power up ADC L)
     *   PMPFIL = 1 (programmable filter)
     */
    Codec_WriteReg(AK4642_REG_PM1, 0x45);
    DelayUs(30000);     /* Wait for ADC power up */

    /* Power Management 3:
     *   PMADR = 1 (power up ADC R)
     *   MDIF1 = 1 (stereo microphone differential input)
     */
    Codec_WriteReg(AK4642_REG_PM3, 0x01);
    DelayUs(10000);
}

void CODEC_Deinitialize(void)
{
    /* Power down all blocks */
    Codec_WriteReg(AK4642_REG_PM1, 0x00);
    Codec_WriteReg(AK4642_REG_PM2, 0x00);
    Codec_WriteReg(AK4642_REG_PM3, 0x00);

    /* Assert power down */
    CODEC_PDN_SetLow();
    CODEC_RESET_SetLow();
}

void CODEC_SetVolume(uint8_t volume)
{
    /* AK4642 digital volume: 0x00 = +12dB, 0xFF = mute
     * Map 0-255 input so that 0=mute, 255=max
     */
    currentVolume = 255 - volume;
    Codec_WriteReg(AK4642_REG_L_DIG_VOL, currentVolume);
    Codec_WriteReg(AK4642_REG_R_DIG_VOL, currentVolume);
}

uint8_t CODEC_GetVolume(void)
{
    return 255 - currentVolume;
}

void CODEC_SetMute(bool mute)
{
    uint8_t val;
    Codec_ReadReg(AK4642_REG_MODE_CTRL3, &val);
    if(mute)
        val |= 0x20;   /* SMUTE = 1 */
    else
        val &= ~0x20;  /* SMUTE = 0 */
    Codec_WriteReg(AK4642_REG_MODE_CTRL3, val);
}

void CODEC_SetInputSource(codec_input_t source)
{
    uint8_t sigSel1;
    uint8_t pm1;

    Codec_ReadReg(AK4642_REG_SIG_SEL1, &sigSel1);
    Codec_ReadReg(AK4642_REG_PM1, &pm1);

    switch(source)
    {
        case CODEC_INPUT_LINE:
            sigSel1 &= ~0x01;  /* PMMP = 0, mic amp off */
            pm1 |= 0x05;       /* PMADL + PMPFIL */
            break;

        case CODEC_INPUT_MIC:
            sigSel1 |= 0x01;   /* PMMP = 1, mic amp on */
            sigSel1 |= 0x04;   /* MGAIN = +20dB for microphone */
            pm1 |= 0x05;
            break;

        default:
            break;
    }

    Codec_WriteReg(AK4642_REG_SIG_SEL1, sigSel1);
    Codec_WriteReg(AK4642_REG_PM1, pm1);
    DelayUs(5000);
}

void CODEC_SetSampleRate(uint32_t sampleRate)
{
    uint8_t modeCtrl2;
    Codec_ReadReg(AK4642_REG_MODE_CTRL2, &modeCtrl2);
    modeCtrl2 &= 0xF0;     /* Clear FS bits */

    switch(sampleRate)
    {
        case 8000:  modeCtrl2 |= 0x00; break;
        case 12000: modeCtrl2 |= 0x01; break;
        case 16000: modeCtrl2 |= 0x02; break;
        case 24000: modeCtrl2 |= 0x03; break;
        case 11025: modeCtrl2 |= 0x05; break;
        case 22050: modeCtrl2 |= 0x07; break;
        case 32000: modeCtrl2 |= 0x06; break;
        case 48000: modeCtrl2 |= 0x05; break;  /* 256fs mode */
        case 44100: modeCtrl2 |= 0x0F; break;
        default:    modeCtrl2 |= 0x05; break;  /* Default 48 kHz */
    }

    Codec_WriteReg(AK4642_REG_MODE_CTRL2, modeCtrl2);
}
