#ifndef FINAL_CONTROLLER_H
#define FINAL_CONTROLLER_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h> 
#include "i2c.h"

// Pre-scaler macros
#define PRSC_8      _BV(CS11)               // used to generate ctrl signals
#define PRSC_64     _BV(CS11)|_BV(CS10)     // used to measure radio signals

// Bounds on radio input signals
#define MIN_TICKS   222     //  888 uS at 1/64
#define MAX_TICKS   477     // 1908 uS at 1/64

// Since the clock speed is 16MHz, and the pre-scaler is 1/8 (in non-manual mode),
// TWO clock ticks equals ONE microsecond.
#define MAX_PULSE_TICKS		4000    //  2 ms
#define PERIOD_TICKS		44000   // 22 ms

// Numeric codes for PYRT
enum {
    pitch = 0,
    yaw,
    roll,
    throttle
} portcode;

// States used for generating CTRL signals when in non-manual mode
typedef enum {
    pulseP, syncP,
    pulseY, syncY,
    pulseR, syncR,
    pulseT, syncT
} outstate_t;

outstate_t ctrlState;

// CTRL_* outputs
volatile uint16_t syncTime;     // remaining time until next rising edge
volatile uint16_t ctrlLen[4];   // length of pulses in clk ticks for each channel

// RADIO_* inputs
volatile uint8_t oldPORTD;
volatile uint16_t oldClk[4];    // saved clock tick of last rising edge
volatile uint8_t pulseLen[4];   // pulse length in units of 4uS, relative to MIN_TICKS

#endif