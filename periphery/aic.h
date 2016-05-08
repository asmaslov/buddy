#ifndef AIC_H
#define AIC_H

extern void AIC_ConfigureIT(unsigned int source,
                            unsigned int mode,
                            void (*handler)( void ));

extern void AIC_EnableIT(unsigned int source);

extern void AIC_DisableIT(unsigned int source);

extern void AIC_ClearIT(unsigned int source);

extern void AIC_FinishIT(void);

#endif //#ifndef AIC_H
