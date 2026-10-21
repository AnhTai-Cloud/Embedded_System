#ifndef TEST_TIMERS_H
#define TEST_TIMERS_H
#include "FreeRTOS.h"
typedef void *TimerHandle_t;
typedef void (*TimerCallbackFunction_t)(TimerHandle_t timer);
TimerHandle_t xTimerCreate(const char *name, TickType_t ticks, BaseType_t reload,
                          void *id, TimerCallbackFunction_t callback);
BaseType_t xTimerReset(TimerHandle_t timer, TickType_t block_time);
BaseType_t xTimerResetFromISR(TimerHandle_t timer, BaseType_t *woken);
#endif
