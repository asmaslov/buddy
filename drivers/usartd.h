#ifndef USARTD_H
#define USARTD_H

#include "pio.h"

/// USART0 RXD pin definition.
#define PIN_USART0_RXD  {BIT0, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/// USART0 TXD pin definition.
#define PIN_USART0_TXD  {BIT1, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/// USART0 SCK pin definition.
#define PIN_USART0_SCK  {BIT2, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/// USART0 RTS pin definition
#define PIN_USART0_RTS  {BIT3, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/// USART0 CTS pin definition
#define PIN_USART0_CTS  {BIT4, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}

#define PINS_USART0 PIN_USART0_RXD, PIN_USART0_TXD, PIN_USART0_SCK, PIN_USART0_RTS, PIN_USART0_CTS
#define PINS_USART0_RXTX PIN_USART0_RXD, PIN_USART0_TXD

/// USART1 RXD pin definition.
#define PIN_USART1_RXD  {BIT5, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/// USART1 TXD pin definition.
#define PIN_USART1_TXD  {BIT6, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/// USART1 SCK pin definition.
#define PIN_USART1_SCK  {BIT7, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/// USART1 RTS pin definition
#define PIN_USART1_RTS  {BIT8, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/// USART1 CTS pin definition
#define PIN_USART1_CTS  {BIT9, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}

#define PINS_USART0 PIN_USART0_RXD, PIN_USART0_TXD, PIN_USART0_SCK, PIN_USART0_RTS, PIN_USART0_CTS
#define PINS_USART0_RXTX PIN_USART0_RXD, PIN_USART0_TXD

#define PINS_USART1 PIN_USART1_RXD, PIN_USART1_TXD, PIN_USART1_SCK, PIN_USART1_RTS, PIN_USART1_CTS
#define PINS_USART1_RXTX PIN_USART1_RXD, PIN_USART1_TXD

static const Pin USART0_pins[] = {
  PINS_USART0_RXTX,
};

static const Pin USART1_pins[] = {
  PINS_USART1,
};

class USARTDriver
{
  public:
    USARTDriver();
    ~USARTDriver();
};

#endif //#ifndef USARTD_H