#include "board_memories.h"
#include "board.h"

#define BOARD_FLASH 0
#define BOARD_RAM 1

static unsigned char BOARD_GetRemap(void)
{
  unsigned int *remap = (unsigned int *) 0;
  unsigned int *ram = (unsigned int *) AT91C_ISRAM;
  
  // Try to write in 0 and see if this affects the RAM
  unsigned int temp = *ram;
  *ram = temp + 1;
  if (*remap == *ram)
  {
    *ram = temp;
    return BOARD_RAM;
  }
  else
  {
    *ram = temp;
    return BOARD_FLASH;
  }
}

void BOARD_RemapFlash( void )
{
  if (BOARD_GetRemap() != BOARD_FLASH)
  {
    AT91C_BASE_MC->MC_RCR = AT91C_MC_RCB;
  }
}

void BOARD_RemapRam( void )
{
  if (BOARD_GetRemap() != BOARD_RAM)
  {
    AT91C_BASE_MC->MC_RCR = AT91C_MC_RCB;
  }
}
