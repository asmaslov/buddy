#ifndef BOARD_H 
#define BOARD_H

#include "AT91SAM7X256.h"
#include "bits.h"

/// Frequency of the board main oscillator
#define BOARD_MAINOSC 18432000
/// Master clock frequency
#define BOARD_MCK 48000000

/// ADC clock frequency, at 10-bit resolution (in Hz)
#define ADC_MAX_CK_10BIT 5000000
/// ADC clock frequency, at 8-bit resolution (in Hz)
#define ADC_MAX_CK_8BIT 8000000
/// Startup time max, return from Idle mode (in µs)
#define ADC_STARTUP_TIME_MAX 20
/// Track and hold Acquisition Time min (in ns)
#define ADC_TRACK_HOLD_TIME_MIN 600

/// Chip has a UDP controller
#define BOARD_USB_UDP
/// Indicates the D+ pull-up is always connected
#define BOARD_USB_PULLUP_ALWAYSON
/// Number of endpoints in the USB controller
#define BOARD_USB_NUMENDPOINTS 6
/// Returns the maximum packet size of the given endpoint
#define BOARD_USB_ENDPOINTS_MAXPACKETSIZE(i) ((((i) == 4) || ((i) == 5)) ? 256 : (((i) == 0) ? 8 : 64))
/// Returns the number of FIFO banks for the given endpoint
#define BOARD_USB_ENDPOINTS_BANKS(i) ((((i) == 0) || ((i) == 3)) ? 1 : 2)
/// USB attributes configuration descriptor (bus or self powered, remote wakeup)
#define BOARD_USB_BMATTRIBUTES USBConfigurationDescriptor_BUSPOWERED_NORWAKEUP

//------------------------------------------------------------------------------
/// \page "SAM7X-EK - PIO definitions"
/// This pages lists all the pio definitions contained in board.h. The constants
/// are named using the following convention: PIN_* for a constant which defines
/// a single Pin instance (but may include several PIOs sharing the same
/// controller), and PINS_* for a list of Pin instances.
///
/// !DBGU
/// - PINS_DBGU
/// 
/// !LEDs
/// - PIN_LED_0
/// - PIN_LED_1
/// - PIN_LED_2
/// - PIN_LED_3
/// - PINS_LEDS
/// 
/// !Push buttons
/// - PIN_PUSHBUTTON_1
/// - PIN_PUSHBUTTON_2
/// - PIN_PUSHBUTTON_3
/// - PIN_PUSHBUTTON_4
/// - PINS_PUSHBUTTONS
/// - PUSHBUTTON_BP1
/// - PUSHBUTTON_BP2
/// - PUSHBUTTON_BP3
/// - PUSHBUTTON_BP4
/// - LED_DS1
/// - LED_DS2
/// - LED_DS3
/// - LED_DS4
///
/// !Joystick buttons
/// - PIN_JOYSTICK_UP
/// - PIN_JOYSTICK_DOWN
/// - PIN_JOYSTICK_LEFT
/// - PIN_JOYSTICK_RIGHT
/// - PIN_JOYSTICK_LCLIC, PIN_JOYSTICK_PUSH
/// - PINS_JOYSTICK_MOVE, PINS_JOYSTICK_CLIC, PINS_JOYSTICK
/// - JOYSTICK_UP
/// - JOYSTICK_DOWN
/// - JOYSTICK_LEFT
/// - JOYSTICK_RIGHT
/// - JOYSTICK_LCLIC, JOYSTICK_PUSH
/// 
/// !USART0
/// - PIN_USART0_RXD
/// - PIN_USART0_TXD
/// - PIN_USART0_SCK
/// 
/// !SPI0
/// - PIN_SPI0_MISO
/// - PIN_SPI0_MOSI
/// - PIN_SPI0_SPCK
/// - PINS_SPI0
/// - PIN_SPI0_NPCS0
/// - PIN_SPI0_NPCS1
///
/// !PWMC
/// - PIN_PWMC_PWM0
/// - PIN_PWMC_PWM1
/// - PIN_PWMC_PWM2
/// - PIN_PWMC_PWM3
/// - PIN_PWM_LED0
/// - PIN_PWM_LED1
/// - CHANNEL_PWM_LED0
/// - CHANNEL_PWM_LED1
/// 
/// !ADC
/// - PIN_ADC_ADC0
/// - PIN_ADC_ADC1
/// - PIN_ADC_ADC2
/// - PIN_ADC_ADC3
/// - PINS_ADC
///
/// !TWI
/// - PINS_TWI
///
/// !CAN
/// - PIN_CAN_TRANSCEIVER_RS
/// - PINS_CAN_TRANSCEIVER_TXD
/// - PINS_CAN_TRANSCEIVER_RXD

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
#define JOYSTICK_UP         0
#define JOYSTICK_DOWN       1
#define JOYSTICK_LEFT       2
#define JOYSTICK_RIGHT      3
#define JOYSTICK_BUTTON     4

/// SPI0 MISO pin definition.
#define PIN_SPI0_MISO   {1 << 16, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/// SPI0 MOSI pin definition.
#define PIN_SPI0_MOSI   {1 << 17, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_PULLUP}
/// SPI0 SPCK pin definition.
#define PIN_SPI0_SPCK   {1 << 18, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_PULLUP}
/// List of SPI0 pin definitions (MISO, MOSI & SPCK).
#define PINS_SPI0       PIN_SPI0_MISO, PIN_SPI0_MOSI, PIN_SPI0_SPCK
/// SPI0 chip select 0 pin definition.
#define PIN_SPI0_NPCS0  {1 << 12, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_PULLUP}
/// SPI0 chip select 1 pin definition.
#define PIN_SPI0_NPCS1  {1 << 13, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_PULLUP}

/// PWMC PWM0 pin definition.
#define PIN_PWMC_PWM0  {1 << 19, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
/// PWMC PWM1 pin definition.
#define PIN_PWMC_PWM1  {1 << 20, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
/// PWMC PWM2 pin definition.
#define PIN_PWMC_PWM2  {1 << 21, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
/// PWMC PWM3 pin definition.
#define PIN_PWMC_PWM3  {1 << 22, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}

/// ADC_AD0 pin definition.
#define PIN_ADC0_ADC0 {1 << 27, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEFAULT}
/// ADC_AD1 pin definition.
#define PIN_ADC0_ADC1 {1 << 28, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEFAULT}
/// ADC_AD2 pin definition.
#define PIN_ADC0_ADC2 {1 << 29, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEFAULT}
/// ADC_AD3 pin definition.
#define PIN_ADC0_ADC3 {1 << 30, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEFAULT}
/// Pins ADC
#define PINS_ADC PIN_ADC0_ADC0, PIN_ADC0_ADC1, PIN_ADC0_ADC2, PIN_ADC0_ADC3

/// PWM pin definition for LED0
#define PIN_PWM_LED0 PIN_PWMC_PWM1
/// PWM pin definition for LED1
#define PIN_PWM_LED1 PIN_PWMC_PWM2
/// PWM channel for LED0
#define CHANNEL_PWM_LED0 1
/// PWM channel for LED1
#define CHANNEL_PWM_LED1 2

/// TWI pins definition.
#define PINS_TWI  {0x00000C00, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}

/// CAN Definition
/// RS: Select input for high speed mode or silent mode
#define PIN_CAN_TRANSCEIVER_RS  {1<<2, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
/// TXD: Transmit data input
#define PINS_CAN_TRANSCEIVER_TXD  {1<<20, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/// RXD: Receive data output
#define PINS_CAN_TRANSCEIVER_RXD  {1<<19, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}

#endif //#ifndef BOARD_H

