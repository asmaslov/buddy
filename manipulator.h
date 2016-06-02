#ifndef MANIPULATOR_H
#define MANIPULATOR_H

#include "comvault.h"
#include "commander.h"

#define FORWARD 0
#define BACK    1

#define PIN_CLOCK_X {BIT22, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define PIN_CLOCK_Y {BIT27, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define PIN_CLOCK_ZR {BIT3, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define PIN_CLOCK_ZL {BIT4, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define PINS_CLOCKS PIN_CLOCK_X, PIN_CLOCK_Y, PIN_CLOCK_ZR, PIN_CLOCK_ZL

#define CLOCK_MAX_FREQ_HZ 300000
#define CLOCK_FREQ_HZ      10000
#define MATH_FREQ_HZ         100
#define PARSER_FREQ_HZ        10

#define I2C_MIN_PERIOD_US   1000
#define I2C_PERIOD_US       5000

#define SPEED_CALIBRATE_MULTIPLIED -100

#define SPEED_MIN 1
#define SPEED_TEST 10
#define SPEED_MAX 100
#define ACCELERATION 8
#define DECELERATION_DIVIDER 4
#define ZERO_GAP 105
#define HALF_DEAD_ZONE 5

#define STEP_DIVIDER 16

#define CONTROL_SPEED 0
#define CONTROL_POS   1

typedef struct {
  bit moving;
  bit sensZeroPos;
  int realPos;
  int oldPos;
  union {
    unsigned short reqPos;
    struct {
      unsigned char reqPosL;
      unsigned char reqPosH;
    };
  };
  unsigned short maxPos;
  int realSpeed;
  int reqSpeed;
  bit limitTopSpeed;
  unsigned char topSpeed;
  unsigned int maxSpeed;
  unsigned int maxAccel;
  unsigned int clockFreq;
  unsigned char direction;
  bit inverted;
  SoftwareTimer timer;
} Joint;

typedef struct {
  Joint joints[TOTAL_JOINTS];
  bit globalMotorsTickersEnabled;
  unsigned int globalSpeedMultiplier;
  unsigned char control;
  bit calibrated;
  unsigned int realx;
  unsigned int realy;
  unsigned int realzr;
  unsigned int realzl;
  bit busy;
} Manipulator;

void manipulator_init(Manipulator *m,
                      Commander *c,
                      CommandVault *cv,
                      Comport *cp);

void manipulator_configure(CommanderTicker ct);

void manipulator_startParser(void);

void manipulator_stopParser(void);

void manipulator_unfreeze(void);

void manipulator_freeze(void);

#endif // MANIPULATOR_H
