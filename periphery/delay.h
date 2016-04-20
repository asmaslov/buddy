#ifndef DELAY_H
#define DELAY_H

void UTIL_Loop(unsigned int loop);

void UTIL_WaitTimeInMs(unsigned int mck,
                       unsigned int time_ms);

void UTIL_WaitTimeInUs(unsigned int mck,
                       unsigned int time_us);

#endif //#ifndef DELAY_H
