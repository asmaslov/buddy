#include "manipulator.h"
#include "pio.h"
#include "tc.h"
#include "aic.h"

#include "smplmath.h"
#include "assert.h"

#include <stdlib.h>

static Manipulator *manipulator;
static CommandVault *commandVault;
static Commander *commander;

static SoftwareTimer mathTimer;

static CommanderTicker commanderTicker;

static const Pin Clocks_pins[] = { PINS_CLOCKS };

static bit allJointsConnected = FALSE;
static bit allInPlace = FALSE;

static void mainTimerHandler(void)
{
  SANITY_CHECK(manipulator);
  SANITY_CHECK(commander);
  SANITY_CHECK(commandVault);
  // Clear status bit to acknowledge interrupt
  unsigned int dummy;
  dummy = AT91C_BASE_TC0->TC_SR;
  dummy = dummy;
  if(commander->timer.enabled)
  {
    if(++commander->timer.tick >= commander->timer.compare)
    {
      commander->timer.tick = 0;
      // Commander timer interrupt
      if(++commander->timer.mastertick >= commander->timer.divide)
      {
        commander->timer.mastertick = 0;
        commanderTicker();
      }
     if(!commandVault_locked()) 
     {
       commandVault_lock();
       manipulator->joints[JOINT_X].sensZeroPos = ((commandVault->status.stat12 & (1 << 7)) == 0) ? TRUE : FALSE;
       manipulator->joints[JOINT_Y].sensZeroPos = ((commandVault->status.stat12 & (1 << 6)) == 0) ? TRUE : FALSE;
       manipulator->joints[JOINT_ZL].sensZeroPos = ((commandVault->status.stat34 & (1 << 6)) == 0) ? TRUE : FALSE;
       manipulator->joints[JOINT_ZR].sensZeroPos = ((commandVault->status.stat34 & (1 << 7)) == 0) ? TRUE : FALSE;
       commandVault_unlock();
     }
    }
    allJointsConnected = TRUE;
    for(int i = 0; i < commander->totalNods; i++)
    {
      allJointsConnected &= commander->nods[i].connected;
    }
    if(manipulator->globalMotorsTickersEnabled && allJointsConnected)
    {
      for(int i = 0; i < TOTAL_JOINTS; i++)
      {
        if(manipulator->joints[i].timer.enabled)
        {
          if(++manipulator->joints[i].timer.tick >= manipulator->joints[i].timer.compare)
          {
            manipulator->joints[i].timer.tick = 0;
            // Motor timer interrupt
            if(++manipulator->joints[i].timer.mastertick >= manipulator->joints[i].timer.divide)
            {
              manipulator->joints[i].timer.mastertick = 0;
              PIO_Invert(&Clocks_pins[i]);
              if(manipulator->joints[i].direction == FORWARD)
              {
                if(!manipulator->joints[i].inverted)
                {
                  manipulator->joints[i].realPos++;
                }
                else
                {
                  manipulator->joints[i].realPos--;
                }                  
              }
              else
              {
                if(!manipulator->joints[i].inverted)
                {
                  manipulator->joints[i].realPos--;
                }
                {
                  manipulator->joints[i].realPos++;
                }
              }
            }
          }
        }
      }  
    }    
  }
  if(mathTimer.enabled)
  {
    if(++mathTimer.tick >= mathTimer.compare)
    {
      mathTimer.tick = 0;
      if(++mathTimer.mastertick >= mathTimer.divide)
      {
        mathTimer.mastertick = 0;
        // Math timer interrupt
        // Top priority command
        if(commandVault->requests.stopAll)
        {
          for(int i = 0; i < TOTAL_JOINTS; i++)
          {
             manipulator->joints[i].reqSpeed = 0;
          }
          manipulator->control = CONTROL_SPEED;
          commandVault->requests.stopAll = FALSE;
        }  
        for(int i = 0; i < TOTAL_JOINTS; i++)
        {
          // TODO:
          // Calculate real speeds
          // and set 'moving' bits
        }
        if(commandVault->requests.newIns)
        {  
          manipulator->control = CONTROL_POS;
          commandVault->requests.newIns = FALSE;
        }
        switch (manipulator->control)
        {
          case CONTROL_SPEED:
            manipulator->joints[JOINT_X].reqSpeed = commandVault->values.speedX * manipulator->globalSpeedPercentage;
            // TODO:
            // if(manipulator->joints[JOINT_X].reqSpeed > manipulator->joints[JOINT_X].maxSpeed)
            manipulator->joints[JOINT_Y].reqSpeed = commandVault->values.speedY * manipulator->globalSpeedPercentage;
            manipulator->joints[JOINT_ZL].reqSpeed = commandVault->values.speedZL * manipulator->globalSpeedPercentage;          
            manipulator->joints[JOINT_ZR].reqSpeed = commandVault->values.speedZR * manipulator->globalSpeedPercentage;
          break;
          case CONTROL_POS:
            switch (commandVault->requests.instruction)
            {
              case INSTRUCTION_CALIBRATE:
                for(int i = 0; i < TOTAL_JOINTS; i++)
                {
                  if(!manipulator->joints[i].sensZeroPos)
                  {
                    manipulator->joints[i].reqSpeed = -CALIBRATE_SPEED;
                  }
                  else
                  {
                    manipulator->joints[i].reqSpeed = 0;
                    manipulator->joints[i].realPos = 0;
                  }
                }
                allInPlace = TRUE;
                for(int i = 0; i < TOTAL_JOINTS; i++)
                {
                  allInPlace &= manipulator->joints[i].sensZeroPos;
                }
                if(allInPlace)
                {
                  manipulator->control = CONTROL_SPEED;
                  manipulator->calibrated = TRUE;
                  TRACE_DEBUG("Manipulator calibration complete\n\r");
                  // TODO:
                  // Emulate command to move to (ZERO_GAP, ZERO_GAP, [ZERO_GAP, ZERO_GAP])
                  commandVault->requests.instruction = INSTRUCTION_GOTO;
                  commandVault->requests.parameters[0] = JOINT_XYZLZR;
                  commandVault->requests.parameters[1] = ZERO_GAP;
                  commandVault->requests.parameters[2] = 0;
                  commandVault->requests.parameters[3] = ZERO_GAP;
                  commandVault->requests.parameters[4] = 0;
                  commandVault->requests.parameters[5] = ZERO_GAP;
                  commandVault->requests.parameters[6] = 0;
                  commandVault->requests.parameters[7] = ZERO_GAP;
                  commandVault->requests.parameters[8] = 0;
                  commandVault->requests.newIns = TRUE;
                }
              break;
              case INSTRUCTION_GOTO:
                /*if(!manipulator->calibrated)
                {
                  TRACE_DEBUG("Manipulator not calibrated\n\r");
                  manipulator->control = CONTROL_SPEED;
                }
                else*/
                {
                  switch (commandVault->requests.parameters[0])
                  {
                    case JOINT_X:
                      // TODO:
                      // if <= manipulator->joints[0].maxPos
                      manipulator->joints[JOINT_X].reqPosL = commandVault->requests.parameters[1];
                      manipulator->joints[JOINT_X].reqPosH = commandVault->requests.parameters[2];
                    break;
                    case JOINT_Y:
                      manipulator->joints[JOINT_Y].reqPosL = commandVault->requests.parameters[1];
                      manipulator->joints[JOINT_Y].reqPosH = commandVault->requests.parameters[2];
                    break;
                    case JOINT_ZL:
                      manipulator->joints[JOINT_ZL].reqPosL = commandVault->requests.parameters[1];
                      manipulator->joints[JOINT_ZL].reqPosH = commandVault->requests.parameters[2];
                    break;
                    case JOINT_ZR:
                      manipulator->joints[JOINT_ZR].reqPosL = commandVault->requests.parameters[1];
                      manipulator->joints[JOINT_ZR].reqPosH = commandVault->requests.parameters[2];
                    break;
                    case JOINT_XY:
                      manipulator->joints[JOINT_X].reqPosL = commandVault->requests.parameters[1];
                      manipulator->joints[JOINT_X].reqPosH = commandVault->requests.parameters[2];
                      manipulator->joints[JOINT_Y].reqPosL = commandVault->requests.parameters[3];
                      manipulator->joints[JOINT_Y].reqPosH = commandVault->requests.parameters[4];
                    break;
                    case JOINT_XYZL:
                      manipulator->joints[JOINT_X].reqPosL = commandVault->requests.parameters[1];
                      manipulator->joints[JOINT_X].reqPosH = commandVault->requests.parameters[2];
                      manipulator->joints[JOINT_Y].reqPosL = commandVault->requests.parameters[3];
                      manipulator->joints[JOINT_Y].reqPosH = commandVault->requests.parameters[4];
                      manipulator->joints[JOINT_ZL].reqPosL = commandVault->requests.parameters[5];
                      manipulator->joints[JOINT_ZL].reqPosH = commandVault->requests.parameters[6];
                    break;
                    case JOINT_XYZR:
                      manipulator->joints[JOINT_X].reqPosL = commandVault->requests.parameters[1];
                      manipulator->joints[JOINT_X].reqPosH = commandVault->requests.parameters[2];
                      manipulator->joints[JOINT_Y].reqPosL = commandVault->requests.parameters[3];
                      manipulator->joints[JOINT_Y].reqPosH = commandVault->requests.parameters[4];
                      manipulator->joints[JOINT_ZR].reqPosL = commandVault->requests.parameters[5];
                      manipulator->joints[JOINT_ZR].reqPosH = commandVault->requests.parameters[6];
                    break;
                    case JOINT_XYZLZR:
                      manipulator->joints[JOINT_X].reqPosL = commandVault->requests.parameters[1];
                      manipulator->joints[JOINT_X].reqPosH = commandVault->requests.parameters[2];
                      manipulator->joints[JOINT_Y].reqPosL = commandVault->requests.parameters[3];
                      manipulator->joints[JOINT_Y].reqPosH = commandVault->requests.parameters[4];
                      manipulator->joints[JOINT_ZL].reqPosL = commandVault->requests.parameters[5];
                      manipulator->joints[JOINT_ZL].reqPosH = commandVault->requests.parameters[6];
                      manipulator->joints[JOINT_ZR].reqPosL = commandVault->requests.parameters[7];
                      manipulator->joints[JOINT_ZR].reqPosH = commandVault->requests.parameters[8];
                    break;
                  }
                }
                for(int i = 0; i < TOTAL_JOINTS; i++)
                {
                  // TODO:
                  // Calculate reqSpeeds from reqPositions with regulators and stuff                 
                  if(manipulator->joints[i].reqPos < manipulator->joints[i].realPos - DEAD_ZONE / 2)
                  {
                    manipulator->joints[i].reqSpeed = -TEST_SPEED;
                  }
                  else if(manipulator->joints[i].reqPos > manipulator->joints[i].realPos + DEAD_ZONE / 2)
                  {
                    manipulator->joints[i].reqSpeed = TEST_SPEED;
                  }
                  else
                  {
                    manipulator->joints[i].reqSpeed = 0;
                  }
                }
                //
                // Breakpoint here
                //
                allInPlace = TRUE;
                for(int i = 0; i < TOTAL_JOINTS; i++)
                {
                  if(abs(manipulator->joints[i].realPos - manipulator->joints[i].reqPos) < DEAD_ZONE)
                  {
                    manipulator->joints[i].reqSpeed = 0;
                  }
                  else
                  {
                    allInPlace = FALSE;
                  }
                }
                if(allInPlace)
                {
                  manipulator->control = CONTROL_SPEED;
                  commandVault->status.instructionDone = TRUE;
                  TRACE_DEBUG("Manipulator command complete\n\r");
                }                
              break;
              default:
                for(int i = 0; i < TOTAL_JOINTS; i++)
                {
                  manipulator->joints[i].reqSpeed = 0;
                }
            }
          break;
        }          
        // Sets step and directions
        for(int i = 0; i < TOTAL_JOINTS; i++)
        {
          if(!manipulator->joints[i].inverted)
          {
            manipulator->joints[i].direction = (manipulator->joints[i].reqSpeed >= 0) ? FORWARD : BACK;
          }
          else
          {
            manipulator->joints[i].direction = (manipulator->joints[i].reqSpeed >= 0) ? BACK : FORWARD;
          }
          if(manipulator->joints[i].reqSpeed == 0)
          {
            manipulator->joints[i].timer.enabled = FALSE;
          }
          else
          {
            double jointFreqHz = abs(manipulator->joints[i].reqSpeed) * STEP_DIVIDER * manipulator->globalSpeedPercentage;
            while((0xFFFFFFFE / (jointFreqHz * manipulator->joints[i].timer.divide)) < CLOCK_FREQ_HZ)
            {
              manipulator->joints[i].timer.divide++;    
            }
            manipulator->joints[i].timer.compare = (unsigned int)(CLOCK_FREQ_HZ / (jointFreqHz * manipulator->joints[i].timer.divide));
            manipulator->joints[i].timer.enabled = TRUE;
          }
        }
        commandVault_lock();
        if(manipulator->joints[JOINT_X].direction == FORWARD)
        {
          // TODO:
          // outputs.endirXX must be calculated
          commandVault->outputs.endir12 &=~(1 << 1);
        }
        else
        {
          commandVault->outputs.endir12 |= (1 << 1);
        }
        if(manipulator->joints[JOINT_Y].direction == FORWARD)
        {
          commandVault->outputs.endir12 &=~(1 << 3);
        }
        else
        {
          commandVault->outputs.endir12 |= (1 << 3);
        }
        if(manipulator->joints[JOINT_ZL].direction == FORWARD)
        {
          commandVault->outputs.endir34 &=~(1 << 1);
        }
        else
        {
          commandVault->outputs.endir34 |= (1 << 1);
        }
        if(manipulator->joints[JOINT_ZR].direction == FORWARD)
        {
          commandVault->outputs.endir34 &=~(1 << 3);
        }
        else
        {
          commandVault->outputs.endir34 |= (1 << 3);
        }
        commandVault_unlock(); 
      }
    }
  }
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
  manipulator->globalSpeedPercentage = 0;
  manipulator->control = CONTROL_SPEED;
  manipulator->calibrated = FALSE;
  manipulator->realx = 0;
  manipulator->realy = 0;
  manipulator->realzr = 0;
  manipulator->realzl = 0;
  manipulator->busy = FALSE;
  manipulator->globalMotorsTickersEnabled = FALSE;
  mathTimer.enabled = FALSE;
  for(int i = 0; i < TOTAL_JOINTS; i++)
  {
    manipulator->joints[i].moving = FALSE;
    manipulator->joints[i].sensZeroPos = FALSE;
    manipulator->joints[i].realPos = 0;
    manipulator->joints[i].reqPos = 0;
    manipulator->joints[i].maxPos = 0;
    manipulator->joints[i].realSpeed = 0;
    manipulator->joints[i].reqSpeed = 0;
    manipulator->joints[i].maxSpeed = 0;
    manipulator->joints[i].clockFreq = 0;
    manipulator->joints[i].direction = FORWARD;
    manipulator->joints[i].inverted = FALSE;
    manipulator->joints[i].timer.enabled = FALSE;
    manipulator->joints[i].timer.tick = 0;
    manipulator->joints[i].timer.compare = 0;
    manipulator->joints[i].timer.mastertick = 0;
    manipulator->joints[i].timer.divide = 1;
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
  while((0xFFFFFFFE / (i2cFreqHz * commander->timer.divide)) < CLOCK_FREQ_HZ)
  {
    commander->timer.divide++;    
  }
  commander->timer.compare = (unsigned int)(CLOCK_FREQ_HZ / (i2cFreqHz * commander->timer.divide));  
  TRACE_DEBUG("Commander I2C timer frequency = %d Hz\n\r", (unsigned int)i2cFreqHz);
  while((0xFFFFFFFE / (MATH_FREQ_HZ * mathTimer.divide)) < CLOCK_FREQ_HZ)
  {
    mathTimer.divide++;    
  }
  TRACE_DEBUG("Math timer frequency = %d Hz\n\r", MATH_FREQ_HZ);
  // TODO:
  // Define this somewhere else
  manipulator->joints[JOINT_X].inverted = TRUE;
  manipulator->joints[JOINT_Y].inverted = TRUE;
  manipulator->joints[JOINT_ZR].inverted = TRUE;
}

void manipulator_unfreeze(void)
{
  commandVault_lock();
  commandVault->outputs.endir12 |= (1 << 0);
  commandVault->outputs.endir12 |= (1 << 2);
  commandVault->outputs.endir34 |= (1 << 0);
  commandVault->outputs.endir34 |= (1 << 2);
  commandVault_unlock();
  manipulator->globalMotorsTickersEnabled = TRUE;
  mathTimer.enabled = TRUE;
}

void manipulator_freeze(void)
{
  commandVault_lock();
  commandVault->outputs.endir12 &=~(1 << 0);
  commandVault->outputs.endir12 &=~(1 << 2);
  commandVault->outputs.endir34 &=~(1 << 0);
  commandVault->outputs.endir34 &=~(1 << 2);
  commandVault_unlock();  
  manipulator->globalMotorsTickersEnabled = FALSE;
  mathTimer.enabled = FALSE;
}
