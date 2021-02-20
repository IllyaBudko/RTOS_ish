#ifndef _RTOS_ISH_H_
#define _RTOS_ISH_H_

#include <stdint.h>
#include <stdlib.h>

typedef struct{
  void * sp; // task stack pointer  
  void * stk_alloc; // bottom of stack pointer
  uint32_t timeout; // timeout for blocking delay
  uint8_t priority; // task priority
}TaskTCB;

void OS_Init(void);
void OS_Run(void);

void OS_Schedule(void);
void OS_Tick(void);
void OS_Delay(uint32_t ticks);

typedef void (*OSTaskHandler)(void);
void OSTask_Create(TaskTCB *me,OSTaskHandler Task, size_t stkSize);


void Error_Handler(void);
#endif /*_RTOS_ISH_H_*/
