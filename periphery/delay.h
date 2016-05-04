#ifndef DELAY_H
#define DELAY_H

#ifdef __cplusplus
extern "C" {
#endif

  void delayLoop(unsigned int loop);

  void delayMs(unsigned int time_ms);

  void delayUs(unsigned int time_us);

#ifdef __cplusplus
}
#endif

#endif //#ifndef DELAY_H
