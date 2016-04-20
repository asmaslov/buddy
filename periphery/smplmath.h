#ifndef SMPLMATH_H
#define SMPLMATH_H

extern unsigned int min(unsigned int a,
                        unsigned int b);

extern unsigned int absv(signed int value);

#if defined(__ARMCC_VERSION)
  extern unsigned int pow(unsigned int x,
                          unsigned int y);
#else
  extern unsigned int power(unsigned int x,
                            unsigned int y);
#endif

#endif //#ifndef SMPLMATH_H
