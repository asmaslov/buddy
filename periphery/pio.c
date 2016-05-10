#include "pio.h"
#include "aic.h"
#include "assert.h"

static InterruptSource sources[MAX_INTERRUPT_SOURCES];
static unsigned int numSources;

static void PIO_SetPeripheralA(AT91S_PIO *pio,
                               unsigned int mask,
                               unsigned char enablePullUp)
{
  pio->PIO_IDR = mask;
  if (enablePullUp)
  {
    pio->PIO_PPUER = mask;
  }
  else
  {
    pio->PIO_PPUDR = mask;
  }
  pio->PIO_ASR = mask;
  pio->PIO_PDR = mask;
}

static void PIO_SetPeripheralB(AT91S_PIO *pio,
                               unsigned int mask,
                               unsigned char enablePullUp)
{
  pio->PIO_IDR = mask;
  if (enablePullUp)
  {
    pio->PIO_PPUER = mask;
  }
  else
  {
    pio->PIO_PPUDR = mask;
  }
  pio->PIO_BSR = mask;
  pio->PIO_PDR = mask;
}

static void PIO_SetInput(AT91S_PIO *pio,
                         unsigned int mask,
                         unsigned char enablePullUp,
                         unsigned char enableFilter)
{
  pio->PIO_IDR = mask;
  if (enablePullUp)
  {
    pio->PIO_PPUER = mask;
  }
  else
  {
    pio->PIO_PPUDR = mask;
  }
  if (enableFilter)
  {
    pio->PIO_IFER = mask;
  }
  else
  {
    pio->PIO_IFDR = mask;
  }
  pio->PIO_ODR = mask;
  pio->PIO_PER = mask;
}

static void PIO_SetOutput(AT91S_PIO *pio,
                          unsigned int mask,
                          unsigned char defaultValue,
                          unsigned char enableMultiDrive,
                          unsigned char enablePullUp)
{
  pio->PIO_IDR = mask;
  if (enablePullUp)
  {
    pio->PIO_PPUER = mask;
  }
  else
  {
    pio->PIO_PPUDR = mask;
  }
  if (enableMultiDrive)
  {
    pio->PIO_MDER = mask;
  }
  else
  {
    pio->PIO_MDDR = mask;
  }
  if (defaultValue)
  {
    pio->PIO_SODR = mask;
  }
  else
  {
    pio->PIO_CODR = mask;
  }
  pio->PIO_OER = mask;
  pio->PIO_PER = mask;
}

unsigned char PIO_Configure(const Pin *list,
                            unsigned int size)
{
  while (size > 0)
  {
    switch (list->type)
    {
      case PIO_PERIPH_A:
        PIO_SetPeripheralA(list->pio,
                           list->mask,
                           (list->attribute & PIO_PULLUP) ? 1 : 0);
      break;
      case PIO_PERIPH_B:
        PIO_SetPeripheralB(list->pio,
                           list->mask,
                           (list->attribute & PIO_PULLUP) ? 1 : 0);
      break;
      case PIO_INPUT:
        AT91C_BASE_PMC->PMC_PCER = 1 << list->id;
        PIO_SetInput(list->pio,
                     list->mask,
                     (list->attribute & PIO_PULLUP) ? 1 : 0,
                     (list->attribute & PIO_DEGLITCH) ? 1 : 0);
      break;
      case PIO_OUTPUT_0:
      case PIO_OUTPUT_1:
        PIO_SetOutput(list->pio,
                      list->mask,
                      (list->type == PIO_OUTPUT_1),
                      (list->attribute & PIO_OPENDRAIN) ? 1 : 0,
                      (list->attribute & PIO_PULLUP) ? 1 : 0);
      break;
      default:
        return 0;
    }
    list++;
    size--;
  }
  return 1;
}

void PIO_Set(const Pin *pin)
{
  pin->pio->PIO_SODR = pin->mask;
}

void PIO_Clear(const Pin *pin)
{
  pin->pio->PIO_CODR = pin->mask;
}

unsigned char PIO_Get(const Pin *pin)
{
  unsigned int reg;
  if ((pin->type == PIO_OUTPUT_0) || (pin->type == PIO_OUTPUT_1))
  {
    reg = pin->pio->PIO_ODSR;
  }
  else
  {
    reg = pin->pio->PIO_PDSR;
  }
  if ((reg & pin->mask) == 0)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

unsigned char PIO_GetOutputDataStatus(const Pin *pin)
{
  if ((pin->pio->PIO_ODSR & pin->mask) == 0)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

unsigned int PIO_GetISR(const Pin *pin)
{
  return (pin->pio->PIO_ISR);
}

static void PIO_InterruptHandler(unsigned int id, AT91S_PIO *pPio)
{
  unsigned int status;
  unsigned int i;
  // Read PIO controller status
  status = pPio->PIO_ISR;
  status &= pPio->PIO_IMR;
  // Check pending events
  if (status != 0)
  {
    TRACE_DEBUG("PIO interrupt on PIO controller #%d\n\r", id);
    // Find triggering source
    i = 0;
    while (status != 0)
    {
      // There cannot be an unconfigured source enabled.
      SANITY_CHECK(i < numSources);
      // Source is configured on the same controller
      if (sources[i].pin->id == id)
      {
        // Source has PIOs whose statuses have changed
        if ((status & sources[i].pin->mask) != 0)
        {
          TRACE_DEBUG("Interrupt source #%d triggered\n\r", i);
          sources[i].handler(sources[i].pin);
          status &= ~(sources[i].pin->mask);
        }
      }
      i++;
    }
  }
}

static void PIO_CommonInterruptHandler(void)
{
  PIO_InterruptHandler(AT91C_ID_PIOA, AT91C_BASE_PIOA);
  PIO_InterruptHandler(AT91C_ID_PIOB, AT91C_BASE_PIOB);
}

void PIO_InitializeInterrupts(unsigned int priority)
{
  TRACE_DEBUG("PIO_Initialize()\n\r");
  SANITY_CHECK((priority & ~AT91C_AIC_PRIOR) == 0);
  // Reset sources
  numSources = 0;
  // Configure PIO interrupt sources A
  TRACE_DEBUG("PIO_Initialize: Configuring PIOA\n\r");
  AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_PIOA;
  AT91C_BASE_PIOA->PIO_ISR;
  AT91C_BASE_PIOA->PIO_IDR = 0xFFFFFFFF;
  AIC_ConfigureIT(AT91C_ID_PIOA, priority, PIO_CommonInterruptHandler);
  AIC_EnableIT(AT91C_ID_PIOA);
  // Configure PIO interrupt sources B
  TRACE_DEBUG("PIO_Initialize: Configuring PIOB\n\r");
  AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_PIOB;
  AT91C_BASE_PIOB->PIO_ISR;
  AT91C_BASE_PIOB->PIO_IDR = 0xFFFFFFFF;
  AIC_ConfigureIT(AT91C_ID_PIOB, priority, PIO_CommonInterruptHandler);
  AIC_EnableIT(AT91C_ID_PIOB);
}

void PIO_ConfigureIt(const Pin *pin,
                     PinHandler handler)
{
  TRACE_DEBUG("PIO_ConfigureIt()\n\r");
  ASSERT(numSources < MAX_INTERRUPT_SOURCES, "-F- PIO_ConfigureIt: Increase MAX_INTERRUPT_SOURCES\n\r");
  // Define new source
  TRACE_DEBUG("PIO_ConfigureIt: Defining new source #%d.\n\r", numSources);
  sources[numSources].pin = pin;
  sources[numSources].handler = handler;
  numSources++;
}

void PIO_EnableIt(const Pin *pPin)
{
  TRACE_DEBUG("PIO_EnableIt()\n\r");
  SANITY_CHECK(pPin);
  #ifndef NOASSERT
    unsigned int i = 0;
    unsigned char found = 0;
    while ((i < numSources) && !found)
    {
      if (sources[i].pin == pPin)
      {
        found = 1;
      }
      i++;
    }
    ASSERT(found, "-F- PIO_EnableIt: Interrupt source has not been configured\n\r");
  #endif
  pPin->pio->PIO_ISR;
  pPin->pio->PIO_IER = pPin->mask;
}

void PIO_DisableIt(const Pin *pPin)
{
  SANITY_CHECK(pPin);
  TRACE_DEBUG("PIO_DisableIt()\n\r");
  pPin->pio->PIO_IDR = pPin->mask;
}
