/**
 * Audio Equalizer Header
 * 
 * @file      equalizer.h
 * @brief     5-band parametric equalizer using cascaded biquad IIR filters
 *
 * Band definitions (center frequencies):
 *   Band 0:  60 Hz  (Sub-bass)
 *   Band 1: 230 Hz  (Bass)
 *   Band 2: 910 Hz  (Mid)
 *   Band 3: 4000 Hz (Upper-mid)
 *   Band 4: 14000 Hz (Treble)
 *
 * Implementation uses Q15 fixed-point biquad (Direct Form II Transposed)
 * for efficient execution on dsPIC33A DSP engine.
 *
 * Ported from dsPIC33F/E: Same algorithm, but uses dsPIC33A
 * __builtin_mulss / __builtin_mulsu for MAC operations.
 */

#ifndef EQUALIZER_H
#define EQUALIZER_H

#include <stdint.h>
#include <stdbool.h>

/* Number of equalizer bands */
#define EQ_NUM_BANDS        5

/* Biquad filter coefficients in Q15 format */
typedef struct {
    int16_t b0;     /* Feedforward coefficient 0 */
    int16_t b1;     /* Feedforward coefficient 1 */
    int16_t b2;     /* Feedforward coefficient 2 */
    int16_t a1;     /* Feedback coefficient 1 (negated) */
    int16_t a2;     /* Feedback coefficient 2 (negated) */
} biquad_coeffs_t;

/* Biquad filter state (delay line) */
typedef struct {
    int32_t w1;     /* State variable 1 */
    int32_t w2;     /* State variable 2 */
} biquad_state_t;

/**
 * @brief Initialize the 5-band equalizer with flat response
 */
void EQ_Initialize(void);

/**
 * @brief Set gain for a specific EQ band
 * @param band Band index (0-4)
 * @param gain Gain level (0 = -12dB cut, 5 = flat, 10 = +12dB boost)
 */
void EQ_SetBandGain(uint8_t band, uint8_t gain);

/**
 * @brief Process a block of mono audio samples through the equalizer
 * @param samples Audio buffer (in-place processing)
 * @param count Number of samples
 */
void EQ_Process(int16_t *samples, uint16_t count);

/**
 * @brief Reset all filter states (clear delay lines)
 */
void EQ_Reset(void);

/**
 * @brief Apply bass shelf boost/cut
 * @param samples Audio buffer (in-place)
 * @param count Number of samples
 * @param level 0-10 (5 = flat)
 */
void Bass_Process(int16_t *samples, uint16_t count, uint8_t level);

/**
 * @brief Apply treble shelf boost/cut
 * @param samples Audio buffer (in-place)
 * @param count Number of samples
 * @param level 0-10 (5 = flat)
 */
void Treble_Process(int16_t *samples, uint16_t count, uint8_t level);

/**
 * @brief Initialize bass/treble filters
 */
void BassTreble_Initialize(void);

/**
 * @brief Reset bass/treble filter states
 */
void BassTreble_Reset(void);

#endif /* EQUALIZER_H */
