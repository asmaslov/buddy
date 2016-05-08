#ifndef PMC_H
#define PMC_H

extern void PMC_DisableProcessorClock(void);

extern void PMC_EnablePeripheral(unsigned int id);

extern void PMC_DisablePeripheral(unsigned int id);

extern void PMC_CPUInIdleMode(void);

extern void PMC_EnableAllPeripherals(void);

extern void PMC_DisableAllPeripherals(void);

extern unsigned int PMC_IsAllPeriphEnabled(void);

extern unsigned int PMC_IsPeriphEnabled(unsigned int id);

#endif //#ifndef PMC_H
