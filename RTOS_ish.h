#ifndef _RTOS_ISH_H_
#define _RTOS_ISH_H_

#include <stdint.h>
#include <stdlib.h>

typedef struct{
  void * sp; // task stack pointer  
  void * stk_alloc;
}TaskTCB;

void OS_Init(void);
void OS_Run(void);

void OS_Schedule(void);

typedef void (*OSTaskHandler)(void);
void OSTask_Create(TaskTCB *me,OSTaskHandler Task, size_t stkSize);


void Error_Handler(void);
#endif /*_RTOS_ISH_H_*/
