#ifndef _RTOS_ISH_H_
#define _RTOS_ISH_H_

#include <stdint.h>
#include <stdlib.h>



typedef struct{
  void * sp; // task stack pointer  
  void * stk_alloc; // bottom of stack pointer
  uint32_t timeout; // timeout for blocking delay
  uint8_t workingPriority; // task priority
  uint8_t setPriority;
}TaskTCB;


void IdleTask_func(void);
void OS_Init(size_t stkSize);
void OS_Run(void);

void OS_Schedule(void);
void OS_Tick(void);
void OS_Delay(uint32_t ticks);

typedef void (*OSTaskHandler)(void);
void OS_Task_Create(TaskTCB *me,OSTaskHandler Task,uint8_t me_priority, size_t stkSize);


void Error_Handler(void);
#include "rtos_mutex.h"
#endif /*_RTOS_ISH_H_*/
