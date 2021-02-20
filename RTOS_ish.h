#ifndef _RTOS_ISH_H_
#define _RTOS_ISH_H_

#include <stdint.h>
#include <stdlib.h>

typedef struct{
  void * sp; // task stack pointer  
  void * stk_alloc;
}TaskTCB;

typedef void (*OSTaskHandler)(void);

void OSTask_Create(TaskTCB *me,OSTaskHandler Task, size_t stkSize);



#endif /*_RTOS_ISH_H_*/
