#include "lcd.h"
#include "bender.h"

AT91PS_PIO    l_pPioA   = AT91C_BASE_PIOA;
AT91PS_PIO    l_pPioB   = AT91C_BASE_PIOB;
AT91PS_SPI    l_pSpi    = AT91C_BASE_SPI0;
AT91PS_PMC    l_pPMC    = AT91C_BASE_PMC;
AT91PS_PDC    l_pPDC    = AT91C_BASE_PDC_SPI0;

void Delaya (volatile unsigned long a) { while (--a!=0); }

void Delay_ (unsigned long a) {

  volatile unsigned long d;
  d=a;
  while (--d!=0);
}

unsigned int i,j;

void LCD_init(void)
{
  PIO_Configure(lcd_pins, PIO_LISTSIZE(lcd_pins));

  // Init SPI0
  // TODO: Generate spi.c spi.h
  //set functionality to pins:
  //port0.12 -> NPCS0
  //port0.16 -> MISO
  //port0.17 -> MOSI
  //port0.18 -> SPCK
  l_pPioA->PIO_PDR = BIT12 | BIT16 | BIT17 | BIT18 | BIT13;
  l_pPioA->PIO_ASR = BIT12 | BIT16 | BIT17 | BIT18 | BIT13;
  l_pPioA->PIO_BSR = 0;

  //enable the clock of SPI 0
  l_pPMC->PMC_PCER = 1 << AT91C_ID_SPI0;

  // Fixed mode
  l_pSpi->SPI_CR      = 0x80;               //SPI Enable, Sowtware reset
  l_pSpi->SPI_CR      = 0x01;               //SPI Enable

  //l_pSpi->SPI_MR      = 0xE0099;           //Master mode, fixed select, disable decoder, FDIV=1 (NxMCK), PCS=1110, loopback
  //l_pSpi->SPI_MR      = 0xE0019;            //Master mode, fixed select, disable decoder, FDIV=1 (NxMCK), PCS=1110,
  //l_pSpi->SPI_MR      = 0xE0011;            //Master mode, fixed select, disable decoder, FDIV=0 (MCK), PCS=1110
  l_pSpi->SPI_MR      = 0x100E0011;            //Master mode, fixed select, disable decoder, FDIV=1 (MCK), PCS=1110

  //l_pSpi->SPI_CSR[0]  = 0x4A02;             //8bit, CPOL=0, ClockPhase=1, SCLK = 200kHz
  //l_pSpi->SPI_CSR[0]  = 0x4A13;             //9bit, CPOL=1, ClockPhase=1, SCLK = 200kHz
  //l_pSpi->SPI_CSR[0]  = 0x4A12;             //9bit, CPOL=0, ClockPhase=1, SCLK = 200kHz
  //l_pSpi->SPI_CSR[0]  = 0x4A11;             //9bit, CPOL=1, ClockPhase=0, SCLK = 200kHz
  //l_pSpi->SPI_CSR[0]  = 0x01011F11;           //9bit, CPOL=1, ClockPhase=0, SCLK = 48Mhz/32*31 = 48kHz
  // work l_pSpi->SPI_CSR[0]  = 0x01010F11;           //9bit, CPOL=1, ClockPhase=0, SCLK = 48Mhz/32*15 = 96kHz
  l_pSpi->SPI_CSR[0]  = 0x01010C11;           //9bit, CPOL=1, ClockPhase=0, SCLK = 48Mhz/32*12 = 125kHz
  //l_pSpi->SPI_CSR[1]  = 0x01010502; // For SPI 1 ???
}

void WriteSpiCommand(unsigned int data)
{
  data = (data & ~0x0100);
  
  // Wait for the transfer to complete
  while((l_pSpi->SPI_SR & AT91C_SPI_TXEMPTY) == 0);

  l_pSpi->SPI_TDR = data;
}

void WriteSpiData(unsigned int data)
{
  data = (data | 0x0100);

  // Wait for the transfer to complete
  while((l_pSpi->SPI_SR & AT91C_SPI_TXEMPTY) == 0);

  l_pSpi->SPI_TDR = data;
}


void Backlight(unsigned char state)
{
  if(state == BKLGHT_LCD_ON)
    l_pPioB->PIO_SODR   = BIT20;    // Set PB20 to HIGH
  else
    l_pPioB->PIO_CODR   = BIT20;    // Set PB20 to LOW
}

void SetContrast(unsigned char contrast)
{
  WriteSpiCommand(VOLCTR);
  WriteSpiData(32+contrast); // contrast
  WriteSpiData(3); // contrast
}

void LCDSettings(void)
{
  // Hardware reset
  LCD_RESET_LOW;
  Delay_(1000);
  LCD_RESET_HIGH;
  Delay_(1000);

  // Display vontrol
  WriteSpiCommand(DISCTL);
//  WriteSpiData(0x03); // no division
//  WriteSpiData(0x23); // 160 line
//  WriteSpiData(0x02); // 2 highlighte line
  WriteSpiData(0x00); // default
  WriteSpiData(0x20); // (32 + 1) * 4 = 132 lines (of which 130 are visible)
  WriteSpiData(0x0a); // default

  // COM scan
  WriteSpiCommand(COMSCN);
  WriteSpiData(0x00);  // Scan 1-80

  // Internal oscilator ON
  WriteSpiCommand(OSCON);

  // wait aproximetly 100ms
  Delay_(10000);

  // Sleep out
  WriteSpiCommand(SLPOUT);

  // Voltage control
  WriteSpiCommand(VOLCTR);
  WriteSpiData(43); // middle value of V1
  WriteSpiData(0x03); // middle value of resistance value

  // Temperature gradient
  WriteSpiCommand(TMPGRD);
  WriteSpiData(0x00); // default

  // Power control
  WriteSpiCommand(PWRCTR);
  WriteSpiData(0x0f);   // referance voltage regulator on, circuit voltage follower on, BOOST ON

  // Normal display
  WriteSpiCommand(DISNOR);

  // Inverse display
  WriteSpiCommand(DISINV);

  // Partial area off
  WriteSpiCommand(PTLOUT);

  // Scroll area set
//  WriteSpiCommand(ASCSET);
//  WriteSpiData(0);
//  WriteSpiData(0);
//  WriteSpiData(40);
//  WriteSpiData(3);

  // Vertical scrool address start
//  WriteSpiCommand(SCSTART);
//  WriteSpiData(0);

  // Data control
  WriteSpiCommand(DATCTL);
  WriteSpiData(0x00); // all inversions off, column direction
  WriteSpiData(0x03); // RGB sequence
  WriteSpiData(0x02); // Grayscale -> 16

  // Page Address set
  WriteSpiCommand(PASET);
  WriteSpiData(0);
  WriteSpiData(131);

  // Page Column set
  WriteSpiCommand(CASET);
  WriteSpiData(0);
  WriteSpiData(131);
}

void LCDWrite130x130bmp(void)
{
  // Display OFF
  // WriteSpiCommand(DISOFF);

  // WRITE MEMORY
  WriteSpiCommand(RAMWR);

  //for(j=0; j<sizeof(bmp)-396; j++) {
  for(j=0; j<25740; j++)
  {
    WriteSpiData(bender[j]);
    //WriteSpiData(0xFF);
  }

  // wait aproximetly 100ms
  Delay_(10000);

  // Display On
  WriteSpiCommand(DISON);	
}
