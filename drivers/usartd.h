#ifndef USARTD_H
#define USARTD_H

#include "pio.h"
#include "async.h"

#define USART0 0
#define PIN_USART0_RXD  {BIT0, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART0_TXD  {BIT1, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART0_SCK  {BIT2, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART0_RTS  {BIT3, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART0_CTS  {BIT4, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PINS_USART0 PIN_USART0_RXD, PIN_USART0_TXD, PIN_USART0_SCK, PIN_USART0_RTS, PIN_USART0_CTS
#define PINS_USART0_RXTX PIN_USART0_RXD, PIN_USART0_TXD

#define USART1 1
#define PIN_USART1_RXD  {BIT5, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART1_TXD  {BIT6, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART1_SCK  {BIT7, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART1_RTS  {BIT8, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART1_CTS  {BIT9, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PINS_USART1 PIN_USART1_RXD, PIN_USART1_TXD, PIN_USART1_SCK, PIN_USART1_RTS, PIN_USART1_CTS
#define PINS_USART1_RXTX PIN_USART1_RXD, PIN_USART1_TXD

//#define MAX_BPS 500
#define USART_BUFFER_SIZE 10
#define TIMEOUT 0

static const Pin USART0_pins[] = {
  PINS_USART0_RXTX,
};

static const Pin USART1_pins[] = {
  PINS_USART1,
};

typedef void (*ParserFunc)(unsigned char *buf, int size);

typedef struct _Comport {
  unsigned char port;
  unsigned char readBuffer[USART_BUFFER_SIZE];
  ParserFunc parser;
} Comport;

void comport_enable(Comport *cp);

void comport_disable(void);

void comport_configure(unsigned char portnum,
                       unsigned int speed);

void comport_setParserFunc(ParserFunc pfunc);

void comport_uputchar(char c);

void comport_uprintf(char *str, ...);

void comport_udmaprintf(char *str, ...);

unsigned char comport_ugetchar(void);

#endif //#ifndef USARTD_H
