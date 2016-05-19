#include "manipulator.h"
#include "pio.h"
#include "tc.h"
#include "aic.h"

#include "smplmath.h"
#include "assert.h"

static Manipulator *manipulator;
static CommandVault *commandVault;
static Commander *commander;

static SoftwareTimer jointsTimers[TOTAL_JOINTS];
static SoftwareTimer mathTimer;

volatile unsigned char motorsTickerEnabled = FALSE;
volatile unsigned char mathTickerEnabled = FALSE;

static CommanderTicker commanderTicker;

static const Pin Clocks_pins[] = { PINS_CLOCKS };

static void mainTimerHandler(void)
{
  SANITY_CHECK(manipulator);
  SANITY_CHECK(commander);
  // Clear status bit to acknowledge interrupt
  unsigned int dummy;
  dummy = AT91C_BASE_TC0->TC_SR;
  dummy = dummy;
  if(commander->tickerEnabled)
  {
    if(++commander->timer.tick >= commander->timer.compare)
    {
      commander->timer.tick = 0;
      if(++commander->timer.mastertick >= commander->timer.divide)
      {
        commander->timer.mastertick = 0;
        commanderTicker();
      }
    }
  }
  if(motorsTickerEnabled)
  {
    for(int i = 0; i < TOTAL_JOINTS; i++)
    {
      if(++jointsTimers[i].tick >= jointsTimers[i].compare)
      {
        jointsTimers[i].tick = 0;
        if(++jointsTimers[i].mastertick >= jointsTimers[i].divide)
        {
          jointsTimers[i].mastertick = 0;
          PIO_Invert(&Clocks_pins[i]);
        }
      }
    }  
  }
  if(mathTickerEnabled)
  {
    if(++mathTimer.tick >= mathTimer.compare)
    {
      mathTimer.tick = 0;
      if(++mathTimer.mastertick >= mathTimer.divide)
      {
        mathTimer.mastertick = 0;
        // TODO:
        // All mighty calculations will be here
      }
    }
  }
  /*if(go_right)
  {
    step++;
    if(step > STEP_MAX - STEP_MIN)
    {
      koeff = KOEFF_END;
    }
    else if(step > STEP_MAX - (2 * STEP_MIN))
    {
      koeff = KOEFF_SLOW;
    }
    else      
    {
      koeff = KOEFF_FAST;
    }
    if(step > STEP_MAX)
    {
      go_right = FALSE;
      go_left = TRUE;
      commandVault_lock();
      commandVault->requests.endir12 &= ~(1 << 1);
      commandVault->requests.endir12 &= ~(1 << 3);
      commandVault->requests.endir34 &= ~(1 << 1);
      commandVault->requests.endir34 |= (1 << 3);
      commandVault_unlock();
    }
  }
  if(go_left)
  {
    step--;
    if(step < STEP_MIN + STEP_MIN)
    {
      koeff = KOEFF_END;
    }
    else if(step < STEP_MIN + (2 * STEP_MIN))
    {
      koeff = KOEFF_SLOW;
    }      
    else      
    {
      koeff = KOEFF_FAST;
    }
    if(step < STEP_MIN)
    {
      go_right = TRUE;
      go_left = FALSE;
      commandVault_lock();
      commandVault->requests.endir12 |= (1 << 1);
      commandVault->requests.endir12 |= (1 << 3);
      commandVault->requests.endir34 |= (1 << 1);
      commandVault->requests.endir34 &= ~(1 << 3);
      commandVault_unlock();
    }
  }*/
}

static void manipulator_enableTimer(void)
{
    unsigned int div = 1;
    unsigned int tcclks = 0;
    unsigned int multiplicator = 1;
    TRACE_WARNING("Manipulator uses Timer Counter to operate\n\r");
    AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_TC0;
    while(((BOARD_MCK / div) / AT91C_BASE_TC0->TC_RC) != CLOCK_FREQ_HZ)
    {
      TC_FindMckDivisor(CLOCK_FREQ_HZ * multiplicator, BOARD_MCK, &div, &tcclks);
      TC_Configure(AT91C_BASE_TC0, tcclks | AT91C_TC_CPCTRG);
      AT91C_BASE_TC0->TC_RC = (BOARD_MCK / div) / CLOCK_FREQ_HZ;
      multiplicator++;
    }
    TRACE_DEBUG("Timer Counter multiplicator = %d\n\r", multiplicator);
    TRACE_DEBUG("Maximum peripherial frequency = %d Hz\n\r", (BOARD_MCK / div) / AT91C_BASE_TC0->TC_RC);
    AIC_ConfigureIT(AT91C_ID_TC0, AT91C_AIC_PRIOR_HIGHEST, mainTimerHandler);
    AT91C_BASE_TC0->TC_IER = AT91C_TC_CPCS;
    AIC_EnableIT(AT91C_ID_TC0);
    TC_Start(AT91C_BASE_TC0);
}

void manipulator_init(Manipulator *m, Commander *c, CommandVault *cv)
{
  SANITY_CHECK(m);
  manipulator = m;
  SANITY_CHECK(c);
  commander = c;
  SANITY_CHECK(cv);
  commandVault = cv;
  for(int i = 0; i < TOTAL_JOINTS; i++)
  {
    manipulator->joints[i].moving = FALSE;
    manipulator->joints[i].sensZeroPos = FALSE;
    manipulator->joints[i].realPos = 0;
    manipulator->joints[i].maxPos = 0;
    manipulator->joints[i].realSpeed = 0;
    manipulator->joints[i].maxSpeed = 0;
    manipulator->joints[i].clockFreq = 0;
    jointsTimers[i].tick = 0;
    jointsTimers[i].compare = 0;
    jointsTimers[i].mastertick = 0;
    jointsTimers[i].divide = 1;
    mathTimer.tick = 0;
    mathTimer.compare = 0;
    mathTimer.mastertick = 0;
    mathTimer.divide = 1;
  } 
  PIO_Configure(Clocks_pins,  PIO_LISTSIZE(Clocks_pins));
  manipulator_enableTimer();
}

void manipulator_configure(CommanderTicker ct)
{
  SANITY_CHECK(ct);
  commanderTicker = ct;
  SANITY_CHECK(commander)
  double i2cFreqHz = 1000000 / I2C_PERIOD_US;
  while( (0xFFFFFFFE / (i2cFreqHz * commander->timer.divide)) < CLOCK_FREQ_HZ )
  {
    commander->timer.divide++;    
  }
  commander->timer.compare = (unsigned int)(CLOCK_FREQ_HZ / (i2cFreqHz * commander->timer.divide));  
  TRACE_DEBUG("Commander I2C timer frequency = %d Hz\n\r", (unsigned int)i2cFreqHz);
  while( (0xFFFFFFFE / (MATH_FREQ_HZ * mathTimer.divide)) < CLOCK_FREQ_HZ )
  {
    mathTimer.divide++;    
  }
  TRACE_DEBUG("Math timer frequency = %d Hz\n\r", MATH_FREQ_HZ);
  //TRACE_DEBUG("Timer Multiplicator = %d\n\r", multiplicator);
  // TODO:
  // 
  // Calculate compare values for each joint 
  /*for(int i = 0; i < TOTAL_JOINTS; i++)
  {
    jointsTimers[i].compare = CLOCK_MAX_FREQ / manipulator->joints[i].clockFreq;
  }*/
}

void manipulator_unfreeze(void)
{
  commandVault_lock();
  commandVault->requests.endir12 |= (1 << 0);
  commandVault->requests.endir12 |= (1 << 2);
  commandVault->requests.endir34 |= (1 << 0);
  commandVault->requests.endir34 |= (1 << 2);
  commandVault->requests.endir12 |= (1 << 1);
  commandVault->requests.endir12 |= (1 << 3);
  commandVault->requests.endir34 |= (1 << 1);
  commandVault->requests.endir34 &= ~(1 << 3);
  commandVault_unlock();
  motorsTickerEnabled = TRUE;
  commander->tickerEnabled = TRUE;
  mathTickerEnabled = TRUE;
}

void manipulator_freeze(void)
{
  commandVault_lock();
  commandVault->requests.endir12 &=~(1 << 0);
  commandVault->requests.endir12 &=~(1 << 2);
  commandVault->requests.endir34 &=~(1 << 0);
  commandVault->requests.endir34 &=~(1 << 2);
  commandVault_unlock();  
  motorsTickerEnabled = FALSE;
  commander->tickerEnabled = FALSE;
  mathTickerEnabled = FALSE;
}
