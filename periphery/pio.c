#include "pio.h"

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
                     (list->attribute & PIO_PULLUP) ? 1 : 0, (list->attribute & PIO_DEGLITCH)? 1 : 0);
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
