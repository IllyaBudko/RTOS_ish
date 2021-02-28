#ifndef _RTOS_MUTEX_H_
#define _RTOS_MUTEX_H_

#include "RTOS_ish.h"

typedef struct
{
  uint32_t lockToken;
  
}mutex_t;

typedef struct
{
  uint32_t availableList;
  mutex_t * mutexFreeList[32];

}mutex_controller_t;

mutex_t * OS_mutex_create(void);
void OS_mutexFreeList_create(void);

uint32_t asm_set_mutex(mutex_t * mutex);
uint32_t asm_reset_mutex(mutex_t * mutex);

#endif /*_RTOS_MUTEX_H_*/
