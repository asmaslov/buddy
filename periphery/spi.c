#include "spi.h"

void SPI_Enable(AT91S_SPI *spi)
{
  spi->SPI_CR = AT91C_SPI_SPIEN;
}

void SPI_Disable(AT91S_SPI *spi)
{
  spi->SPI_CR = AT91C_SPI_SPIDIS;
}

void SPI_Configure(AT91S_SPI *spi,
                   unsigned int id,
                   unsigned int configuration)
{
  AT91C_BASE_PMC->PMC_PCER = 1 << id;
  spi->SPI_CR = AT91C_SPI_SPIDIS;
  // Execute a software reset of the SPI twice
  spi->SPI_CR = AT91C_SPI_SWRST;
  spi->SPI_CR = AT91C_SPI_SWRST;
  spi->SPI_MR = configuration;
}

void SPI_ConfigureNPCS(AT91S_SPI *spi,
                       unsigned int npcs,
                       unsigned int configuration)
{
  spi->SPI_CSR[npcs] = configuration;
}

void SPI_Write(AT91S_SPI *spi,
               unsigned int npcs,
               unsigned short data)
{
  // Discard contents of RDR register
  //volatile unsigned int discard = spi->SPI_RDR;

  // Send data
  while ((spi->SPI_SR & AT91C_SPI_TXEMPTY) == 0);
  spi->SPI_TDR = data | SPI_PCS(npcs);
  while ((spi->SPI_SR & AT91C_SPI_TDRE) == 0);
}

unsigned char SPI_WriteBuffer(AT91S_SPI *spi,
                              void *buffer,
                              unsigned int length)
{
  // Check if first bank is free
  if (spi->SPI_TCR == 0) {

      spi->SPI_TPR = (unsigned int) buffer;
      spi->SPI_TCR = length;
      spi->SPI_PTCR = AT91C_PDC_TXTEN;
      return 1;
  }
  // Check if second bank is free
  else if (spi->SPI_TNCR == 0) {

      spi->SPI_TNPR = (unsigned int) buffer;
      spi->SPI_TNCR = length;
      return 1;
  }    
  // No free banks
  return 0;
}

unsigned char SPI_IsFinished(AT91S_SPI *pSpi)
{
    return ((pSpi->SPI_SR & AT91C_SPI_TXEMPTY) != 0);
}

unsigned short SPI_Read(AT91S_SPI *spi)
{
    while ((spi->SPI_SR & AT91C_SPI_RDRF) == 0);
    return spi->SPI_RDR & 0xFFFF;
}

unsigned char SPI_ReadBuffer(AT91S_SPI *spi,
                             void *buffer,
                             unsigned int length)
{
  // Check if the first bank is free
  if (spi->SPI_RCR == 0) {

      spi->SPI_RPR = (unsigned int) buffer;
      spi->SPI_RCR = length;
      spi->SPI_PTCR = AT91C_PDC_RXTEN;
      return 1;
  }
  // Check if second bank is free
  else if (spi->SPI_RNCR == 0) {

      spi->SPI_RNPR = (unsigned int) buffer;
      spi->SPI_RNCR = length;
      return 1;
  }

  // No free bank
  return 0;
}
