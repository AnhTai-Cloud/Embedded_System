#ifndef FREERTOS_BOARD_H
#define FREERTOS_BOARD_H

#include "stm32f1xx.h"
#include <stdint.h>

#define BOARD_CPU_HZ 64000000UL

typedef enum
{
    BOARD_FAULT_CLOCK = 1,
    BOARD_FAULT_ALLOC,
    BOARD_FAULT_STACK,
    BOARD_FAULT_ASSERT,
    BOARD_FAULT_SCHEDULER,
    BOARD_FAULT_CREATE
} BoardFault;

extern volatile uint32_t board_fault_code;

/* HSI/2 x 16 = 64 MHz; APB1 = 32 MHz; APB2 = 64 MHz. */
void Board_Init(void);
void Board_Panic(BoardFault fault) __attribute__((noreturn));

#endif
