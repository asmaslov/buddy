#ifndef LCD_H
#define LCD_H

#include "pio.h"

#define BKLGHT_LCD_ON  0x01
#define BKLGHT_LCD_OFF 0x02
#define DISON     0xAF      // Display on
#define DISOFF    0xAE      // Display off
#define DISNOR    0xA6      // Normal display
#define DISINV    0xA7      // Inverse display
#define COMSCN    0xBB      // Common scan direction
#define DISCTL    0xCA      // Display control
#define SLPIN     0x95      // Sleep in
#define SLPOUT    0x94      // Sleep out
#define PASET     0x75      // Page address set
#define CASET     0x15      // Column address set
#define DATCTL    0xBC      // Data scan direction, etc.
#define RGBSET8   0xCE      // 256-color position set
#define RAMWR     0x5C      // Writing to memory
#define RAMRD     0x5D      // Reading from memory
#define PTLIN     0xA8      // Partial display in
#define PTLOUT    0xA9      // Partial display out
#define RMWIN     0xE0      // Read and modify write
#define RMWOUT    0xEE      // End
#define ASCSET    0xAA      // Area scroll set
#define SCSTART   0xAB      // Scroll start set
#define OSCON     0xD1      // Internal oscillation on
#define OSCOFF    0xD2      // Internal oscillation off
#define PWRCTR    0x20      // Power control
#define VOLCTR    0x81      // Electronic contrast control
#define VOLUP     0xD6      // Increment electronic control by 1
#define VOLDOWN   0xD7      // Decrement electronic control by 1
#define TMPGRD    0x82      // Temperature gradient set
#define EPCTIN    0xCD      // Control EEPROM
#define EPCOUT    0xCC      // Cancel EEPROM control
#define EPMWR     0xFC      // Write into EEPROM
#define EPMRD     0xFD      // Read from EEPROM
#define EPSRRD1   0x7C      // Read register 1
#define EPSRRD2   0x7D      // Read register 2
#define NOP       0x25      // NOP instruction

#define PIN_LCD_BACKLIGHT {BIT20, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_OUTPUT_1, PIO_DEFAULT}
#define PIN_LCD_RESET {BIT2, AT91C_BASE_PIOA, AT91C_ID_PIOB, PIO_OUTPUT_1, PIO_DEFAULT}
#define PIN_LCD_CS  {BIT12, AT91C_BASE_PIOA, AT91C_ID_PIOB, PIO_OUTPUT_1, PIO_DEFAULT}
#define PINS_LCD PIN_LCD_BACKLIGHT, PIN_LCD_RESET

static const Pin lcd_pins[] = {
  PINS_LCD
};

#define SPI_SR_TXEMPTY

#define LCD_RESET_LOW     l_pPioA->PIO_CODR   = BIT2
#define LCD_RESET_HIGH    l_pPioA->PIO_SODR   = BIT2

void LCD_init(void);

void LCDSettings(void);

void LCDWrite130x130bmp(void);

void Backlight(unsigned char state);

void WriteSpiCommand(unsigned int data);
void WriteSpiData(unsigned int data);




void SetContrast(unsigned char contrast);

#endif //#ifndef LCD_H
