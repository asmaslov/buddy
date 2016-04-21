#ifndef SPI_H
#define SPI_H

#include "board.h"

/// Calculate the PCS field value given the chip select NPCS value
#define SPI_PCS(npcs) ((~(1 << npcs) & 0xF) << 16)
/// Calculates the value of the CSR SCBR field given the baudrate and MCK.
#define SPI_SCBR(baudrate, masterClock) ((unsigned int) (masterClock / baudrate) << 8)
/// Calculates the value of the CSR DLYBS field given the desired delay (in ns)
#define SPI_DLYBS(delay, masterClock) ((unsigned int) (((masterClock / 1000000) * delay) / 1000) << 16)
/// Calculates the value of the CSR DLYBCT field given the desired delay (in ns)
#define SPI_DLYBCT(delay, masterClock) ((unsigned int) (((masterClock / 1000000) * delay) / 32000) << 16)

#ifdef __cplusplus
extern "C" {
#endif

  extern void SPI_Enable(AT91S_SPI *spi);

  extern void SPI_Disable(AT91S_SPI *spi);

  extern void SPI_Configure(AT91S_SPI *spi,
                            unsigned int id,
                            unsigned int configuration);

  extern void SPI_ConfigureNPCS(AT91S_SPI *spi,
                                unsigned int npcs,
                                unsigned int configuration);

  extern void SPI_Write(AT91S_SPI *spi,
                        unsigned int npcs,
                        unsigned short data);

  extern unsigned char SPI_WriteBuffer(AT91S_SPI *spi,
                                       void *buffer,
                                       unsigned int length);

  extern unsigned char SPI_IsFinished(AT91S_SPI *pSpi);

  extern unsigned short SPI_Read(AT91S_SPI *spi);

  extern unsigned char SPI_ReadBuffer(AT91S_SPI *spi,
                                      void *buffer,
                                      unsigned int length);

#ifdef __cplusplus
}
#endif

#endif //#ifndef SPI_H
