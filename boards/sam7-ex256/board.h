#ifndef BOARD_H 
#define BOARD_H

#include "AT91SAM7X256.h"
#include "bits.h"

// Frequency of the board main oscillator
#define BOARD_MAINOSC 18432000
// Master clock frequency
#define BOARD_MCK 48000000

// Chip has a UDP controller
#define BOARD_USB_UDP
// Indicates the D+ pull-up is always connected
#define BOARD_USB_PULLUP_ALWAYSON
// Number of endpoints in the USB controller
#define BOARD_USB_NUMENDPOINTS 6
// Returns the maximum packet size of the given endpoint
#define BOARD_USB_ENDPOINTS_MAXPACKETSIZE(i) ((((i) == 4) || ((i) == 5)) ? 256 : (((i) == 0) ? 8 : 64))
// Returns the number of FIFO banks for the given endpoint
#define BOARD_USB_ENDPOINTS_BANKS(i) ((((i) == 0) || ((i) == 3)) ? 1 : 2)
// USB attributes configuration descriptor (bus or self powered, remote wakeup)
#define BOARD_USB_BMATTRIBUTES USBConfigurationDescriptor_BUSPOWERED_NORWAKEUP

#define PIN_PUSHBUTTON_1 {BIT24, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEGLITCH | PIO_PULLUP}
#define PIN_PUSHBUTTON_2 {BIT25, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEGLITCH | PIO_PULLUP}
#define PINS_PUSHBUTTONS PIN_PUSHBUTTON_1, PIN_PUSHBUTTON_2
#define PUSHBUTTON_BP1   0
#define PUSHBUTTON_BP2   1

#define PIN_JOYSTICK_UP    {BIT9, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_INPUT, PIO_PULLUP}
#define PIN_JOYSTICK_DOWN  {BIT8, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_INPUT, PIO_PULLUP}
#define PIN_JOYSTICK_LEFT  {BIT7, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_INPUT, PIO_PULLUP}
#define PIN_JOYSTICK_RIGHT {BIT14, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_INPUT, PIO_PULLUP}
#define PIN_JOYSTICK_BUTTON {BIT15, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_INPUT, PIO_PULLUP}
#define PINS_JOYSTICK_MOVE PIN_JOYSTICK_UP, PIN_JOYSTICK_DOWN, PIN_JOYSTICK_LEFT, PIN_JOYSTICK_RIGHT
#define PINS_JOYSTICK PINS_JOYSTICK_MOVE, PIN_JOYSTICK_BUTTON
#define JOYSTICK_UP     0
#define JOYSTICK_DOWN   1
#define JOYSTICK_LEFT   2
#define JOYSTICK_RIGHT  3
#define JOYSTICK_BUTTON 4

// TODO: Put further definitions inside driver
// CAN Definition
// RS: Select input for high speed mode or silent mode
#define PIN_CAN_TRANSCEIVER_RS {BIT2, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
// TXD: Transmit data input
#define PINS_CAN_TRANSCEIVER_TXD {BIT20, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
// RXD: Receive data output
#define PINS_CAN_TRANSCEIVER_RXD {BIT19, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}

#endif //#ifndef BOARD_H
