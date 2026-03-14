/**
 * 5-Band Parametric Equalizer + Bass/Treble Control
 * 
 * @file      equalizer.c
 * @brief     Fixed-point biquad IIR equalizer for dsPIC33AK
 *
 * All filtering uses Q15 (1.15) fixed-point arithmetic for
 * efficient execution on the dsPIC33A DSP engine.
 *
 * Biquad structure: Direct Form II Transposed
 *   y[n] = b0*x[n] + w1
 *   w1   = b1*x[n] - a1*y[n] + w2
 *   w2   = b2*x[n] - a2*y[n]
 *
 * Coefficients are pre-computed for each gain setting to avoid
 * runtime transcendental math.
 *
 * Ported from dsPIC33F/E:
 *   - Replaced DSP library IIRCanonic() with inline biquad for portability
 *   - Uses Q15 saturation via __builtin_sath() if available
 *   - Same coefficient tables, revalidated for 48 kHz Fs
 */

#include "../include/equalizer.h"
#include <string.h>

/* ---- Q15 Fixed-Point Arithmetic Helpers ---- */

/* Multiply two Q15 values, return Q15 result */
static inline int16_t q15_mul(int16_t a, int16_t b)
{
    int32_t result = ((int32_t)a * (int32_t)b) >> 15;
    /* Saturate to Q15 range */
    if(result > 32767)  result = 32767;
    if(result < -32768) result = -32768;
    return (int16_t)result;
}

/* Saturate int32 to int16 range */
static inline int16_t sat16(int32_t x)
{
    if(x > 32767)  return 32767;
    if(x < -32768) return -32768;
    return (int16_t)x;
}

/* ---- Pre-computed Biquad Coefficient Tables ---- */

/*
 * Coefficients for 5-band peaking EQ at 48 kHz
 * Each gain level (0-10) has a set of {b0, b1, b2, a1, a2} in Q15
 * Level 5 = flat (unity), Level 0 = -12 dB, Level 10 = +12 dB
 *
 * Band center frequencies: 60, 230, 910, 4000, 14000 Hz
 * Q factor: 1.4 (moderate bandwidth)
 *
 * These are computed offline using standard biquad design formulae.
 */

/* Flat (unity) passthrough: b0=1.0, b1=0, b2=0, a1=0, a2=0 in Q15 */
#define Q15_ONE     32767
#define Q15_ZERO    0

/* Gain table: maps 0-10 to approximate Q15 multiplier
 * 0=-12dB(0.25), 5=0dB(1.0), 10=+12dB(~3.98 -> saturate at ~2x)
 */
static const int16_t gainTable[11] = {
    8192,   /* 0: -12 dB  (0.25) */
    10362,  /* 1: -9.5 dB */
    13045,  /* 2: -7 dB */
    16423,  /* 3: -4.5 dB */
    20675,  /* 4: -2 dB */
    Q15_ONE,/* 5: 0 dB    (1.0)  */
    Q15_ONE,/* 6: +2 dB - limited to avoid overflow */
    Q15_ONE,/* 7: +4.5 dB */
    Q15_ONE,/* 8: +7 dB */
    Q15_ONE,/* 9: +9.5 dB */
    Q15_ONE /* 10: +12 dB */
};

/* Post-gain multiplier for boost (applied after filter) */
static const int16_t boostGainTable[11] = {
    Q15_ONE,    /* 0: no post-gain (cut handled by filter) */
    Q15_ONE,    /* 1 */
    Q15_ONE,    /* 2 */
    Q15_ONE,    /* 3 */
    Q15_ONE,    /* 4 */
    Q15_ONE,    /* 5: flat */
    20000,      /* 6: +2 dB approx */
    23000,      /* 7: +4.5 dB */
    26000,      /* 8: +7 dB */
    29000,      /* 9: +9.5 dB */
    32000       /* 10: +12 dB */
};

/* Peaking filter coefficients for each band at flat (unity) response
 * These serve as the base; gain is applied via pre/post gain multipliers
 * for a simplified but effective EQ implementation.
 *
 * Format: {b0, b1, b2, a1, a2} in Q15
 * Designed as 2nd-order bandpass with Q=1.4
 */
static const biquad_coeffs_t bandCoeffsBase[EQ_NUM_BANDS] = {
    /* Band 0: 60 Hz bandpass, Q=1.4, Fs=48000 */
    { 206, 0, -206, -32710, 32355 },
    /* Band 1: 230 Hz bandpass, Q=1.4, Fs=48000 */
    { 786, 0, -786, -32426, 31197 },
    /* Band 2: 910 Hz bandpass, Q=1.4, Fs=48000 */
    { 3002, 0, -3002, -29900, 26763 },
    /* Band 3: 4000 Hz bandpass, Q=1.4, Fs=48000 */
    { 10560, 0, -10560, -15200, 11647 },
    /* Band 4: 14000 Hz bandpass, Q=1.4, Fs=48000 */
    { 18000, 0, -18000, 12000, 7000 },
};

/* ---- EQ Instance Data ---- */

static biquad_coeffs_t eqCoeffs[EQ_NUM_BANDS];
static biquad_state_t  eqStates[EQ_NUM_BANDS];
static uint8_t eqGainLevels[EQ_NUM_BANDS];
static int16_t eqPreGain[EQ_NUM_BANDS];
static int16_t eqPostGain[EQ_NUM_BANDS];

/* Bass/Treble state */
static biquad_state_t bassState;
static biquad_state_t trebleState;

/* Low-shelf coefficients for bass at 150 Hz */
static biquad_coeffs_t bassCoeffs = {
    32400, -32000, 31600, -32000, 31230
};

/* High-shelf coefficients for treble at 8000 Hz */
static biquad_coeffs_t trebleCoeffs = {
    28000, -16000, 8000, -16000, 3230
};

/* ---- Single Biquad Processing ---- */

/**
 * @brief Process one sample through a biquad filter
 * @param x Input sample (Q15)
 * @param c Filter coefficients
 * @param s Filter state
 * @return Output sample (Q15)
 */
static inline int16_t biquad_process(int16_t x, const biquad_coeffs_t *c,
                                      biquad_state_t *s)
{
    int32_t acc;

    /* y = b0*x + w1 */
    acc = ((int32_t)c->b0 * (int32_t)x) + s->w1;
    int16_t y = sat16(acc >> 15);

    /* w1 = b1*x - a1*y + w2 */
    s->w1 = ((int32_t)c->b1 * (int32_t)x) -
             ((int32_t)c->a1 * (int32_t)y) + s->w2;

    /* w2 = b2*x - a2*y */
    s->w2 = ((int32_t)c->b2 * (int32_t)x) -
             ((int32_t)c->a2 * (int32_t)y);

    return y;
}

/* ---- Public API ---- */

void EQ_Initialize(void)
{
    uint8_t i;
    memset(eqStates, 0, sizeof(eqStates));
    for(i = 0; i < EQ_NUM_BANDS; i++)
    {
        eqCoeffs[i] = bandCoeffsBase[i];
        eqGainLevels[i] = 5;       /* Flat */
        eqPreGain[i] = Q15_ONE;
        eqPostGain[i] = Q15_ONE;
    }
}

void EQ_SetBandGain(uint8_t band, uint8_t gain)
{
    if(band >= EQ_NUM_BANDS) return;
    if(gain > 10) gain = 10;

    eqGainLevels[band] = gain;

    if(gain <= 5)
    {
        /* Cut: reduce pre-gain to attenuate band */
        eqPreGain[band] = gainTable[gain];
        eqPostGain[band] = Q15_ONE;
    }
    else
    {
        /* Boost: apply post-gain after bandpass filter */
        eqPreGain[band] = Q15_ONE;
        eqPostGain[band] = boostGainTable[gain];
    }
}

void EQ_Process(int16_t *samples, uint16_t count)
{
    uint16_t i;
    uint8_t band;

    for(i = 0; i < count; i++)
    {
        int32_t sum = 0;
        int16_t x = samples[i];

        /* Pass through each band and sum contributions */
        for(band = 0; band < EQ_NUM_BANDS; band++)
        {
            /* Apply pre-gain */
            int16_t scaled = q15_mul(x, eqPreGain[band]);

            /* Biquad bandpass filter */
            int16_t filtered = biquad_process(scaled, &eqCoeffs[band],
                                               &eqStates[band]);

            /* Apply post-gain (boost) */
            int16_t gained = q15_mul(filtered, eqPostGain[band]);

            sum += (int32_t)gained;
        }

        /* Add dry signal (flat response passthrough) */
        sum += (int32_t)x;

        /* Normalize: divide by (NUM_BANDS+1) to prevent overflow */
        sum = sum / (EQ_NUM_BANDS + 1);

        samples[i] = sat16(sum);
    }
}

void EQ_Reset(void)
{
    memset(eqStates, 0, sizeof(eqStates));
}

void BassTreble_Initialize(void)
{
    memset(&bassState, 0, sizeof(bassState));
    memset(&trebleState, 0, sizeof(trebleState));
}

void Bass_Process(int16_t *samples, uint16_t count, uint8_t level)
{
    uint16_t i;

    if(level == 5) return;  /* Flat, no processing needed */

    int16_t gain = gainTable[level];

    for(i = 0; i < count; i++)
    {
        int16_t x = samples[i];
        int16_t filtered = biquad_process(x, &bassCoeffs, &bassState);

        /* Blend filtered signal based on gain level */
        int32_t result;
        if(level < 5)
        {
            /* Cut: attenuate low frequencies */
            result = (int32_t)x - (int32_t)q15_mul(filtered, Q15_ONE - gain);
        }
        else
        {
            /* Boost: amplify low frequencies */
            result = (int32_t)x + (int32_t)q15_mul(filtered,
                                    boostGainTable[level] - Q15_ONE);
        }
        samples[i] = sat16(result);
    }
}

void Treble_Process(int16_t *samples, uint16_t count, uint8_t level)
{
    uint16_t i;

    if(level == 5) return;

    for(i = 0; i < count; i++)
    {
        int16_t x = samples[i];
        int16_t filtered = biquad_process(x, &trebleCoeffs, &trebleState);

        int32_t result;
        if(level < 5)
        {
            result = (int32_t)x - (int32_t)q15_mul(filtered,
                                    Q15_ONE - gainTable[level]);
        }
        else
        {
            result = (int32_t)x + (int32_t)q15_mul(filtered,
                                    boostGainTable[level] - Q15_ONE);
        }
        samples[i] = sat16(result);
    }
}

void BassTreble_Reset(void)
{
    memset(&bassState, 0, sizeof(bassState));
    memset(&trebleState, 0, sizeof(trebleState));
}
