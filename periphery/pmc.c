#include "pmc.h"
#include "board.h"
#include "assert.h"

#ifdef CP15_PRESENT
  #include "cp15.h"
#endif

#define MASK_STATUS 0x3FFFFFFC

void PMC_DisableProcessorClock(void)
{    
  AT91C_BASE_PMC->PMC_SCDR = AT91C_PMC_PCK;   
  while ((AT91C_BASE_PMC->PMC_SCSR & AT91C_PMC_PCK) != AT91C_PMC_PCK); 
}

void PMC_EnablePeripheral(unsigned int id)
{
  SANITY_CHECK(id < 32);
  if ((AT91C_BASE_PMC->PMC_PCSR & (1 << id)) == (1 << id))
  {
    TRACE_INFO("PMC_EnablePeripheral: clock of peripheral %u is already enabled\n\r", id);
  }
  else
  {
    AT91C_BASE_PMC->PMC_PCER = 1 << id;
  }
}

void PMC_DisablePeripheral(unsigned int id)
{
  SANITY_CHECK(id < 32);
  if ((AT91C_BASE_PMC->PMC_PCSR & (1 << id)) != (1 << id))
  {
    TRACE_INFO("PMC_DisablePeripheral: clock of peripheral %u is not enabled\n\r", id);
  }
  else
  {
    AT91C_BASE_PMC->PMC_PCDR = 1 << id;
  }
}

void PMC_EnableAllPeripherals(void)
{
  AT91C_BASE_PMC->PMC_PCER = MASK_STATUS;
  while( (AT91C_BASE_PMC->PMC_PCSR & MASK_STATUS) != MASK_STATUS);
  TRACE_INFO("Enable all periph clocks\n\r"); 
}

void PMC_DisableAllPeripherals(void)
{
  AT91C_BASE_PMC->PMC_PCDR = MASK_STATUS;
  while((AT91C_BASE_PMC->PMC_PCSR & MASK_STATUS) != 0);
  TRACE_INFO("Disable all periph clocks\n\r");
}

unsigned int PMC_IsAllPeriphEnabled(void)
{
  return (AT91C_BASE_PMC->PMC_PCSR == MASK_STATUS);
}

unsigned int PMC_IsPeriphEnabled(unsigned int id)
{
  return (AT91C_BASE_PMC->PMC_PCSR & (1 << id));  
}

void PMC_CPUInIdleMode(void)
{
  PMC_DisableProcessorClock();
  #ifdef CP15_PRESENT
    _waitForInterrupt();
  #endif
}
