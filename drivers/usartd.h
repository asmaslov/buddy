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
#define BUFFER_SIZE 12
#define TIMEOUT 0

static const Pin USART0_pins[] = {
  PINS_USART0_RXTX,
};

static const Pin USART1_pins[] = {
  PINS_USART1,
};

class USARTDriver
{
  private:
    unsigned char port;
    static unsigned char readBuffer[BUFFER_SIZE];
    static unsigned int idx;
    
  private:
    static void defaultISR0(void);
    static void defaultISR1(void);
    
  public:
    USARTDriver();
    ~USARTDriver();
    void configure(unsigned char portnum,
                   unsigned int speed = 115200);
    void uputchar(char c);
    void uprintf(char *str, ...);
    void udmaprintf(char *str, ...);
    unsigned char ugetchar(void);
    static unsigned char ulast(void);
    static void upush(unsigned char c);
    static void upop(void);
};

#endif //#ifndef USARTD_H
