#include "manipulator.h"
#include "pio.h"
#include "tc.h"
#include "aic.h"

#include "assert.h"

static Manipulator *manipulator;
static CommandVault *commandVault;

static SoftwareTimer jointsTimers[TOTAL_JOINTS];
static SoftwareTimer mathTimer;
volatile unsigned char motorsTickerEnable = FALSE;
static const Pin Clocks_pins[] = { PINS_CLOCKS };

static void mainTimerHandler(void)
{
  SANITY_CHECK(manipulator);
  // Clear status bit to acknowledge interrupt
  unsigned int dummy;
  dummy = AT91C_BASE_TC0->TC_SR;
  dummy = dummy;

  
  // TODO:
  if(motorsTickerEnable)
  {
    for(int i = 0; i < TOTAL_JOINTS; i++)
    {
      if(++jointsTimers[i].tick == jointsTimers[i].compare)
      {
        if(++jointsTimers[i].mastertick == jointsTimers[i].divide)
        {
          PIO_Invert(&Clocks_pins[i]);
        }
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
    while(((BOARD_MCK / div) / AT91C_BASE_TC0->TC_RC) != CLOCK_MAX_FREQ)
    {
      TC_FindMckDivisor(CLOCK_MAX_FREQ * multiplicator, BOARD_MCK, &div, &tcclks);
      TC_Configure(AT91C_BASE_TC0, tcclks | AT91C_TC_CPCTRG);
      AT91C_BASE_TC0->TC_RC = (BOARD_MCK / div) / CLOCK_MAX_FREQ;  
      multiplicator++;
    }
    TRACE_DEBUG("Timer Multiplicator = %d\n\r", multiplicator );
    TRACE_DEBUG("Maximum frequency = %d Hz\n\r", (BOARD_MCK / div) / AT91C_BASE_TC0->TC_RC);
    AIC_ConfigureIT(AT91C_ID_TC0, AT91C_AIC_PRIOR_HIGHEST, mainTimerHandler);
    AT91C_BASE_TC0->TC_IER = AT91C_TC_CPCS;
    AIC_EnableIT(AT91C_ID_TC0);
    TC_Start(AT91C_BASE_TC0);
}

void manipulator_init(Manipulator *m, CommandVault *cv)
{
  SANITY_CHECK(m);
  manipulator = m;
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
    jointsTimers[i].divide = 1;
  } 

  PIO_Configure(Clocks_pins,  PIO_LISTSIZE(Clocks_pins));
  manipulator_enableTimer();
}

void manipulator_configure(void)
{
  // TODO:
  // 
  // Calculate compare values for each joint 
  for(int i = 0; i < TOTAL_JOINTS; i++)
  {
    jointsTimers[i].compare = CLOCK_MAX_FREQ / manipulator->joints[i].clockFreq;
  }
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
}

void manipulator_freeze(void)
{
  commandVault_lock();
  commandVault->requests.endir12 &=~(1 << 0);
  commandVault->requests.endir12 &=~(1 << 2);
  commandVault->requests.endir34 &=~(1 << 0);
  commandVault->requests.endir34 &=~(1 << 2);
  commandVault_unlock();  
}
