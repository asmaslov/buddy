#ifndef PIT_H
#define PIT_H
   
#ifdef __cplusplus
extern "C" {
#endif
  
  extern void PIT_Init(unsigned int period,
                       unsigned int pit_frequency);

  extern void PIT_SetPIV(unsigned int piv);

  extern void PIT_Enable(void);

  extern void PIT_EnableIT(void);

  extern void PIT_DisableIT(void);

  extern unsigned int PIT_GetMode(void);

  extern unsigned int PIT_GetStatus(void);

  extern unsigned int PIT_GetPIIR(void);

  extern unsigned int PIT_GetPIVR(void);

#ifdef __cplusplus
}
#endif

#endif //#ifndef PIT_H
