#ifndef AIC_H
#define AIC_H

#ifdef __cplusplus
extern "C" {
#endif

  extern void AIC_ConfigureIT(unsigned int source, unsigned int mode, void (*handler)( void ));

  extern void AIC_EnableIT(unsigned int source);

  extern void AIC_DisableIT(unsigned int source);

#ifdef __cplusplus
}
#endif

#endif //#ifndef AIC_H
