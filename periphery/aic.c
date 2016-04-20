#include "aic.h"
#include "board.h"

void AIC_ConfigureIT(unsigned int source,
                     unsigned int mode,
                     void (*handler)(void))
{
  // Disable the interrupt first
  AT91C_BASE_AIC->AIC_IDCR = 1 << source;
  // Configure mode and handler
  AT91C_BASE_AIC->AIC_SMR[source] = mode;
  AT91C_BASE_AIC->AIC_SVR[source] = (unsigned int) handler;
  // Clear interrupt
  AT91C_BASE_AIC->AIC_ICCR = 1 << source;
}

void AIC_EnableIT(unsigned int source)
{
  AT91C_BASE_AIC->AIC_IECR = 1 << source;
}

void AIC_DisableIT(unsigned int source)
{
  AT91C_BASE_AIC->AIC_IDCR = 1 << source;
}
