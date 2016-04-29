#ifndef DELAY_H
#define DELAY_H

#ifdef __cplusplus
extern "C" {
#endif

  void UTIL_Loop(unsigned int loop);

  void UTIL_WaitTimeInMs(unsigned int mck,
                         unsigned int time_ms);

  void UTIL_WaitTimeInUs(unsigned int mck,
                         unsigned int time_us);

#ifdef __cplusplus
}
#endif

#endif //#ifndef DELAY_H
