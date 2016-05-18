#ifndef MANIPULATOR_H
#define MANIPULATOR_H

#include "comvault.h"

#define TOTAL_JOINTS 4
#define JOINT_X  0
#define JOINT_Y  1
#define JOINT_ZR 2
#define JOINT_ZL 3

#define PIN_CLOCK_X {BIT22, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define PIN_CLOCK_Y {BIT27, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define PIN_CLOCK_ZR {BIT3, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define PIN_CLOCK_ZL {BIT4, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define PINS_CLOCKS PIN_CLOCK_X, PIN_CLOCK_Y, PIN_CLOCK_ZR, PIN_CLOCK_ZL

#define CLOCK_MAX_FREQ 300000 // Hz
#define MATH_FREQ 100 // Hz

#define KOEFF_END 1
#define KOEFF_SLOW 2
#define KOEFF_FAST 5
#define STEP_MAX 2000
#define STEP_MIN 100

typedef struct {
  unsigned char moving; // bool
  unsigned char sensZeroPos; // bool
  unsigned long realPos;
  unsigned long maxPos;
  unsigned long realSpeed;
  unsigned long maxSpeed;
  unsigned int clockFreq;
} Joint;

typedef struct {
  Joint joints[TOTAL_JOINTS];
  unsigned int globalSpeedPercentage;
  unsigned int realx;
  unsigned int realy;
  unsigned int realzr;
  unsigned int realzl;
} Manipulator;

typedef struct {
  unsigned long tick;
  unsigned long compare;
  unsigned int mastertick;
  unsigned int divide;
} SoftwareTimer;

void manipulator_init(Manipulator *m, CommandVault *cv);

void manipulator_configure(void);

void manipulator_unfreeze(void);

void manipulator_freeze(void);

#endif // MANIPULATOR_H
