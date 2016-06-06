#include "manipulator.h"
#include "pio.h"
#include "tc.h"
#include "aic.h"
#include "usartd.h"
#include "parser.h"

#include "smplmath.h"
#include "assert.h"

#include <stdlib.h>

static Manipulator *manipulator;
static CommandVault *commandVault;
static Commander *commander;
static Comport *comport;

Parser parser;

static SoftwareTimer mathTimer;

static CommanderTicker commanderTicker;

static const Pin Clocks_pins[] = { PINS_CLOCKS };

static bit allJointsConnected = FALSE;
static bit allInPlace = FALSE;

static void manipulator_configureJoints(void)
{
  manipulator->joints[JOINT_X].maxSpeed = SPEED_MAX;
  manipulator->joints[JOINT_Y].maxSpeed = SPEED_MAX;
  manipulator->joints[JOINT_ZL].maxSpeed = SPEED_MAX;
  manipulator->joints[JOINT_ZR].maxSpeed = SPEED_MAX;
  manipulator->joints[JOINT_X].maxAccel = ACCELERATION;
  manipulator->joints[JOINT_Y].maxAccel = ACCELERATION;
  manipulator->joints[JOINT_ZL].maxAccel = ACCELERATION;
  manipulator->joints[JOINT_ZR].maxAccel = ACCELERATION;
  /*manipulator->joints[JOINT_X].maxPos = JOINT_X_MAX;
  manipulator->joints[JOINT_Y].maxPos = JOINT_Y_MAX;
  manipulator->joints[JOINT_ZL].maxPos = JOINT_ZL_MAX;
  manipulator->joints[JOINT_ZR].maxPos = JOINT_ZR_MAX;*/
  manipulator->joints[JOINT_X].inverted = TRUE;
  manipulator->joints[JOINT_Y].inverted = TRUE;
  manipulator->joints[JOINT_ZR].inverted = TRUE;  
}

static void processCommands()
{
  // Set initial values based on manual override
  manipulator->joints[JOINT_X].reqSpeed = commandVault->values.speedX * manipulator->globalSpeedMultiplier;
  manipulator->joints[JOINT_Y].reqSpeed = commandVault->values.speedY * manipulator->globalSpeedMultiplier;
  manipulator->joints[JOINT_ZL].reqSpeed = commandVault->values.speedZL * manipulator->globalSpeedMultiplier;          
  manipulator->joints[JOINT_ZR].reqSpeed = commandVault->values.speedZR * manipulator->globalSpeedMultiplier;
  for(int i = 0; i < TOTAL_JOINTS; i++)
  {
    if(abs(manipulator->joints[i].reqSpeed) > manipulator->joints[i].maxSpeed)
    {
      manipulator->joints[i].reqSpeed = sign(manipulator->joints[i].reqSpeed) * manipulator->joints[i].maxSpeed;
    }              
  }
  // Process instruction if any for manipulator movement
  for(int k = 0; k < commandVault->requests.totalInstructions; k++)
  {
    switch(getInstruction(k)->code)
    {
      case INSTRUCTION_STOP_INIT:
        if(getInstruction(k)->condition == INSTRUCTION_STATUS_ACCEPTED)
        {
          getInstruction(k)->condition = INSTRUCTION_STATUS_WORKING;
          if(!manipulator->globalMotorsTickersEnabled)
          {
            manipulator_unfreeze();
            TRACE_INFO("Manipulator motors enabled\n\r");
          }
          for(int i = 0; i < TOTAL_JOINTS; i++)
          {
             manipulator->joints[i].reqSpeed = 0;
          }
          // TODO: Break all other manipulator instructions
        }
        if(getInstruction(k)->condition == INSTRUCTION_STATUS_WORKING)
        {
          allInPlace = TRUE;
          for(int i = 0; i < TOTAL_JOINTS; i++)
          {
            if(abs(manipulator->joints[i].realSpeed) != 0)
            {
              allInPlace = FALSE;
            }
          }
          if(allInPlace)
          {
            getInstruction(k)->condition = INSTRUCTION_STATUS_DONE;
            TRACE_DEBUG("Manipulator stop\n\r");
          } 
        }
      break;
      case INSTRUCTION_CALIBRATE:
        if(getInstruction(k)->condition == INSTRUCTION_STATUS_ACCEPTED)
        {
          getInstruction(k)->condition = INSTRUCTION_STATUS_WORKING;
          manipulator->calibrated = FALSE;
        }
        if(getInstruction(k)->condition == INSTRUCTION_STATUS_WORKING)
        {
          allInPlace = TRUE;
          for(int i = 0; i < TOTAL_JOINTS; i++)
          {
            if(!manipulator->joints[i].sensZeroPos)
            {
              allInPlace = FALSE;
              manipulator->joints[i].reqSpeed = SPEED_CALIBRATE_MULTIPLIED;
            }
            else
            {
              manipulator->joints[i].reqSpeed = 0;
              manipulator->joints[i].realPos = 0;
            }
          }
          if(allInPlace)
          {
            manipulator->calibrated = TRUE;
            getInstruction(k)->condition = INSTRUCTION_STATUS_DONE;
            TRACE_DEBUG("Manipulator calibrated\n\r");
            Instruction newIns;
            newIns.condition = INSTRUCTION_STATUS_ACCEPTED;
            newIns.idx = 0;
            newIns.code = INSTRUCTION_GOTO;
            newIns.parameters[0] = JOINT_XYZLZR;
            newIns.parameters[1] = ZERO_GAP;
            newIns.parameters[2] = 0;
            newIns.parameters[3] = ZERO_GAP;
            newIns.parameters[4] = 0;
            newIns.parameters[5] = ZERO_GAP;
            newIns.parameters[6] = 0;
            newIns.parameters[7] = ZERO_GAP;
            newIns.parameters[8] = 0;
            newIns.parameters[9] = SPEED_GAP;
            newIns.parameters[10] = SPEED_GAP;
            newIns.parameters[11] = SPEED_GAP;
            newIns.parameters[12] = SPEED_GAP;
            addInstruction(&newIns);
          }
        }
      break;
      case INSTRUCTION_GOTO:
        if(getInstruction(k)->condition == INSTRUCTION_STATUS_ACCEPTED)
        {
          if(!manipulator->calibrated)
          {
            getInstruction(k)->condition = INSTRUCTION_STATUS_ERROR;
            TRACE_DEBUG("Manipulator not calibrated\n\r");
          }
          else
          {
            getInstruction(k)->condition = INSTRUCTION_STATUS_WORKING;
            switch (getInstruction(k)->parameters[0])
            {
              case JOINT_X:
                manipulator->joints[JOINT_X].reqPosL = getInstruction(k)->parameters[1];
                manipulator->joints[JOINT_X].reqPosH = getInstruction(k)->parameters[2];
                manipulator->joints[JOINT_X].topSpeed = getInstruction(k)->parameters[3];
              break;
              case JOINT_Y:
                manipulator->joints[JOINT_Y].reqPosL = getInstruction(k)->parameters[1];
                manipulator->joints[JOINT_Y].reqPosH = getInstruction(k)->parameters[2];
                manipulator->joints[JOINT_Y].topSpeed = getInstruction(k)->parameters[3];
              break;
              case JOINT_ZL:
                manipulator->joints[JOINT_ZL].reqPosL = getInstruction(k)->parameters[1];
                manipulator->joints[JOINT_ZL].reqPosH = getInstruction(k)->parameters[2];
                manipulator->joints[JOINT_ZL].topSpeed = getInstruction(k)->parameters[3];
              break;
              case JOINT_ZR:
                manipulator->joints[JOINT_ZR].reqPosL = getInstruction(k)->parameters[1];
                manipulator->joints[JOINT_ZR].reqPosH = getInstruction(k)->parameters[2];
                manipulator->joints[JOINT_ZR].topSpeed = getInstruction(k)->parameters[3];
              break;
              case JOINT_XY:
                manipulator->joints[JOINT_X].reqPosL = getInstruction(k)->parameters[1];
                manipulator->joints[JOINT_X].reqPosH = getInstruction(k)->parameters[2];
                manipulator->joints[JOINT_Y].reqPosL = getInstruction(k)->parameters[3];
                manipulator->joints[JOINT_Y].reqPosH = getInstruction(k)->parameters[4];
                manipulator->joints[JOINT_X].topSpeed = getInstruction(k)->parameters[5];
                manipulator->joints[JOINT_Y].topSpeed = getInstruction(k)->parameters[6];
              break;
              case JOINT_XYZL:
                manipulator->joints[JOINT_X].reqPosL = getInstruction(k)->parameters[1];
                manipulator->joints[JOINT_X].reqPosH = getInstruction(k)->parameters[2];
                manipulator->joints[JOINT_Y].reqPosL = getInstruction(k)->parameters[3];
                manipulator->joints[JOINT_Y].reqPosH = getInstruction(k)->parameters[4];
                manipulator->joints[JOINT_ZL].reqPosL = getInstruction(k)->parameters[5];
                manipulator->joints[JOINT_ZL].reqPosH = getInstruction(k)->parameters[6];
                manipulator->joints[JOINT_X].topSpeed = getInstruction(k)->parameters[7];
                manipulator->joints[JOINT_Y].topSpeed = getInstruction(k)->parameters[8];
                manipulator->joints[JOINT_ZL].topSpeed = getInstruction(k)->parameters[9];
              break;
              case JOINT_XYZR:
                manipulator->joints[JOINT_X].reqPosL = getInstruction(k)->parameters[1];
                manipulator->joints[JOINT_X].reqPosH = getInstruction(k)->parameters[2];
                manipulator->joints[JOINT_Y].reqPosL = getInstruction(k)->parameters[3];
                manipulator->joints[JOINT_Y].reqPosH = getInstruction(k)->parameters[4];
                manipulator->joints[JOINT_ZR].reqPosL = getInstruction(k)->parameters[5];
                manipulator->joints[JOINT_ZR].reqPosH = getInstruction(k)->parameters[6];
                manipulator->joints[JOINT_X].topSpeed = getInstruction(k)->parameters[7];
                manipulator->joints[JOINT_Y].topSpeed = getInstruction(k)->parameters[8];
                manipulator->joints[JOINT_ZR].topSpeed = getInstruction(k)->parameters[9];
              break;
              case JOINT_XYZLZR:
                manipulator->joints[JOINT_X].reqPosL = getInstruction(k)->parameters[1];
                manipulator->joints[JOINT_X].reqPosH = getInstruction(k)->parameters[2];
                manipulator->joints[JOINT_Y].reqPosL = getInstruction(k)->parameters[3];
                manipulator->joints[JOINT_Y].reqPosH = getInstruction(k)->parameters[4];
                manipulator->joints[JOINT_ZL].reqPosL = getInstruction(k)->parameters[5];
                manipulator->joints[JOINT_ZL].reqPosH = getInstruction(k)->parameters[6];
                manipulator->joints[JOINT_ZR].reqPosL = getInstruction(k)->parameters[7];
                manipulator->joints[JOINT_ZR].reqPosH = getInstruction(k)->parameters[8];
                manipulator->joints[JOINT_X].topSpeed = getInstruction(k)->parameters[9];
                manipulator->joints[JOINT_Y].topSpeed = getInstruction(k)->parameters[10];
                manipulator->joints[JOINT_ZL].topSpeed = getInstruction(k)->parameters[11];
                manipulator->joints[JOINT_ZR].topSpeed = getInstruction(k)->parameters[12];
              break;
            }
          }
        }
        if(getInstruction(k)->condition == INSTRUCTION_STATUS_WORKING)
        {
          allInPlace = TRUE;
          for(int i = 0; i < TOTAL_JOINTS; i++)
          {
            if(abs(manipulator->joints[i].realPos - manipulator->joints[i].reqPos) < HALF_DEAD_ZONE)
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
            getInstruction(k)->condition = INSTRUCTION_STATUS_DONE;
            TRACE_DEBUG("Manipulator movement instruction complete\n\r");
          }     
        }            
      break;
    }
  }
  // TODO: Make intelligent cleaner instead of this stupid one
  bit cleanInProgress = TRUE;
  while(cleanInProgress)
  {
    cleanInProgress = FALSE;
    for(int k = 0; k < commandVault->requests.totalInstructions; k++)
    {
      if((getInstruction(k)->condition == INSTRUCTION_STATUS_DONE) ||
         (getInstruction(k)->condition == INSTRUCTION_STATUS_ERROR) ||
         (getInstruction(k)->condition == INSTRUCTION_STATUS_BREAK))
      {
        removeInstruction(k);            
        cleanInProgress = TRUE;
        break;
      }
    }
  }
}

static void regulateSpeeds(void)
{
  for(int i = 0; i < TOTAL_JOINTS; i++)
  {                               
    if(manipulator->joints[i].topSpeed != 0)
    {
      manipulator->joints[i].limitTopSpeed = TRUE;
    }
    else
    {
      manipulator->joints[i].limitTopSpeed = FALSE;                      
    }
    // TODO:
    // Uncommend after #define JOINT_i_MAX
    /*if(manipulator->joints[i].reqPos > manipulator->joints[i].maxPos)
    {
      manipulator->joints[i].reqPos = manipulator->joints[i].maxPos;
    }*/
    // Dead zone sense and speed reference
    if(manipulator->joints[i].reqPos < manipulator->joints[i].realPos - HALF_DEAD_ZONE)
    {
      if(manipulator->joints[i].limitTopSpeed)
      {
        manipulator->joints[i].reqSpeed = -(manipulator->joints[i].topSpeed * manipulator->globalSpeedMultiplier);
      }
      else
      {
        manipulator->joints[i].reqSpeed = -(SPEED_MAX * manipulator->globalSpeedMultiplier);
      }
    }
    else if(manipulator->joints[i].reqPos > manipulator->joints[i].realPos + HALF_DEAD_ZONE)
    {
      if(manipulator->joints[i].limitTopSpeed)
      {
        manipulator->joints[i].reqSpeed = manipulator->joints[i].topSpeed * manipulator->globalSpeedMultiplier;
      }
      else
      {
        manipulator->joints[i].reqSpeed = (SPEED_MAX * manipulator->globalSpeedMultiplier);
      }
    }
    else
    {
      manipulator->joints[i].reqSpeed = 0;
    }
  #ifndef IGNORE_ACCELERATION_LIMITS
    // If not near the end then limit acceleration
    if(abs(manipulator->joints[i].realSpeed) < (abs(manipulator->joints[i].reqPos - manipulator->joints[i].realPos) * manipulator->joints[i].maxAccel / (ACCELERATION * DECELERATION_DIVIDER)))
    {
      if((abs(manipulator->joints[i].reqSpeed) > abs(manipulator->joints[i].realSpeed)) &&
         (abs(manipulator->joints[i].reqSpeed - manipulator->joints[i].realSpeed) > manipulator->joints[i].maxAccel))
      {
        manipulator->joints[i].reqSpeed = sign(manipulator->joints[i].reqSpeed) * (abs(manipulator->joints[i].realSpeed) + manipulator->joints[i].maxAccel);
      }
    }
    // If near the end then begin deceleration
    else
    {
      manipulator->joints[i].reqSpeed = sign(manipulator->joints[i].reqSpeed) * (abs(manipulator->joints[i].reqPos - manipulator->joints[i].realPos) * manipulator->joints[i].maxAccel / (ACCELERATION * DECELERATION_DIVIDER));
    }
  #endif // IGNORE_ACCELERATION_LIMITS
  }       
}

static void setTickersAndDirections()
{
  for(int i = 0; i < TOTAL_JOINTS; i++)
  {
    if(!manipulator->joints[i].inverted)
    {
      manipulator->joints[i].direction = (manipulator->joints[i].reqSpeed < 0) ? BACK : FORWARD;
    }
    else
    {
      manipulator->joints[i].direction = (manipulator->joints[i].reqSpeed < 0) ? FORWARD : BACK;
    }
    if(manipulator->joints[i].reqSpeed == 0)
    {
      manipulator->joints[i].timer.enabled = FALSE;
    }
    else
    {
      double jointFreqHz = abs(manipulator->joints[i].reqSpeed) * STEP_DIVIDER;
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
    // outputs.endirXX must be calculated (XX) according to joint number
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

static void manipulator_handler(void)
{
  SANITY_CHECK(manipulator);
  SANITY_CHECK(commander);
  SANITY_CHECK(commandVault);
  SANITY_CHECK(comport);
  unsigned int dummy;
  dummy = AT91C_BASE_TC0->TC_SR;
  dummy = dummy;
  if(commander->timer.enabled)
  {
    if(++commander->timer.tick >= commander->timer.compare)
    {
      commander->timer.tick = 0;
      if(++commander->timer.mastertick >= commander->timer.divide)
      {
        commander->timer.mastertick = 0;
        // Commander timer interrupt
        commanderTicker();
        if(!commandVault_locked()) 
        {
         commandVault_lock();
         manipulator->joints[JOINT_X].sensZeroPos = ((commandVault->status.stat12 & (1 << 7)) == 0) ? TRUE : FALSE;
         manipulator->joints[JOINT_Y].sensZeroPos = ((commandVault->status.stat12 & (1 << 6)) == 0) ? TRUE : FALSE;
         manipulator->joints[JOINT_ZL].sensZeroPos = ((commandVault->status.stat34 & (1 << 6)) == 0) ? TRUE : FALSE;
         manipulator->joints[JOINT_ZR].sensZeroPos = ((commandVault->status.stat34 & (1 << 7)) == 0) ? TRUE : FALSE;
         commandVault->status.ready = allJointsConnected;
         commandVault_unlock();
        }
        allJointsConnected = TRUE;
        for(int i = 0; i < commander->totalNods; i++)
        {
          allJointsConnected &= commander->nods[i].connected;
        }
      }
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
            if(++manipulator->joints[i].timer.mastertick >= manipulator->joints[i].timer.divide)
            {
              manipulator->joints[i].timer.mastertick = 0;
              // Motor timer interrupt
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
                else
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
  if(parser.timer.enabled)
  {
    if(++parser.timer.tick >= parser.timer.compare)
    {
      parser.timer.tick = 0;
      if(++parser.timer.mastertick >= parser.timer.divide)
      {
        parser.timer.mastertick = 0;
        // Parser timer interrupt
        if(comport->parser)
        {
          comport->parser(comport->readBuffer, USART_BUFFER_SIZE);
        }
        if(commandVault->leftFeedbacks > 0)
        {
          if(!mathTimer.enabled)
          {
            mathTimer.enabled = TRUE;
          }
          commander_replyAuto(commandVault->lastPacketIdx);
          commandVault->leftFeedbacks--;
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
        for(int i = 0; i < TOTAL_JOINTS; i++)
        {
          manipulator->joints[i].realSpeed = (manipulator->joints[i].realPos - manipulator->joints[i].oldPos) * MATH_FREQ_HZ / STEP_DIVIDER;
          manipulator->joints[i].oldPos = manipulator->joints[i].realPos;
          if(manipulator->joints[i].realSpeed != 0)
          {
            manipulator->joints[i].moving = TRUE;
          }
          else
          {
            manipulator->joints[i].moving = FALSE;
          }
        }
        processCommands();
        if(manipulator->calibrated)
        {
          regulateSpeeds();
        }
        setTickersAndDirections();
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
  AIC_ConfigureIT(AT91C_ID_TC0, AT91C_AIC_PRIOR_HIGHEST, manipulator_handler);
  AT91C_BASE_TC0->TC_IER = AT91C_TC_CPCS;
  AIC_EnableIT(AT91C_ID_TC0);
  TC_Start(AT91C_BASE_TC0);
}

void manipulator_init(Manipulator *m, Commander *c, CommandVault *cv, Comport *cp)
{
  SANITY_CHECK(m);
  manipulator = m;
  SANITY_CHECK(c);
  commander = c;
  SANITY_CHECK(cv);
  commandVault = cv;
  SANITY_CHECK(cp);
  comport = cp;
  parser_enable(&parser, commandVault);
  comport_setParserFunc(parser_work);  
  manipulator->globalSpeedMultiplier = 0;
  manipulator->calibrated = FALSE;
  manipulator->realx = 0;
  manipulator->realy = 0;
  manipulator->realzr = 0;
  manipulator->realzl = 0;
  manipulator->busy = FALSE;
  manipulator->globalMotorsTickersEnabled = FALSE;
  mathTimer.enabled = FALSE;
  mathTimer.tick = 0;
  mathTimer.compare = 0;
  mathTimer.mastertick = 0;
  mathTimer.divide = 1;
  for(int i = 0; i < TOTAL_JOINTS; i++)
  {
    manipulator->joints[i].moving = FALSE;
    manipulator->joints[i].sensZeroPos = FALSE;
    manipulator->joints[i].realPos = 0;
    manipulator->joints[i].oldPos = 0;
    manipulator->joints[i].reqPos = 0;
    manipulator->joints[i].maxPos = 0;
    manipulator->joints[i].realSpeed = 0;
    manipulator->joints[i].reqSpeed = 0;
    manipulator->joints[i].maxSpeed = 0;
    manipulator->joints[i].limitTopSpeed = FALSE;
    manipulator->joints[i].topSpeed = 0;
    manipulator->joints[i].maxAccel = 0;
    manipulator->joints[i].clockFreq = 0;
    manipulator->joints[i].direction = FORWARD;
    manipulator->joints[i].inverted = FALSE;
    manipulator->joints[i].timer.enabled = FALSE;
    manipulator->joints[i].timer.tick = 0;
    manipulator->joints[i].timer.compare = 0;
    manipulator->joints[i].timer.mastertick = 0;
    manipulator->joints[i].timer.divide = 1;
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
  mathTimer.compare = (unsigned int)(CLOCK_FREQ_HZ / (MATH_FREQ_HZ * mathTimer.divide));  
  TRACE_DEBUG("Math timer frequency = %d Hz\n\r", MATH_FREQ_HZ);
  while((0xFFFFFFFE / (MATH_FREQ_HZ * parser.timer.divide)) < CLOCK_FREQ_HZ)
  {
    parser.timer.divide++;    
  }
  parser.timer.compare = (unsigned int)(CLOCK_FREQ_HZ / (PARSER_FREQ_HZ * parser.timer.divide));  
  TRACE_DEBUG("Parser timer frequency = %d Hz\n\r", PARSER_FREQ_HZ);
  manipulator_configureJoints();
}

void manipulator_startParser(void)
{
  parser.timer.enabled = TRUE;
}

void manipulator_stopParser(void)
{
  parser.timer.enabled = FALSE;
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
  //mathTimer.enabled = TRUE;
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
  //mathTimer.enabled = FALSE;
}
