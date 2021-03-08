#ifndef _RTOS_MUTEX_H_
#define _RTOS_MUTEX_H_

#include "RTOS_ish.h"

typedef struct
{
  uint32_t  lockToken;
  uint32_t  mutexFreeListIdx;
  uint32_t  mutexWaitingList;
  TaskTCB * mutexOwnerTask;
  
}mutex_t;

typedef struct
{
  uint32_t availableList;
  mutex_t * mutexFreeList[32];

}mutex_controller_t;

/*happens before interrupts are enabled*/
void OS_mutexFreeList_create(void);

mutex_t * OS_mutex_create(void);
void OS_mutex_destroy(mutex_t ** mutex);

/*non-blocking mutex apis*/
uint8_t OS_nonblocking_mutex_lock(mutex_t * mutex);
uint8_t OS_nonblocking_mutex_unlock(mutex_t * mutex);

/*blocking mutex apis*/
void OS_blocking_mutex_lock(mutex_t * mutex);
void OS_blocking_mutex_unlock(mutex_t * mutex);

uint32_t asm_set_mutex(mutex_t *  mutex);    //return 1 for success, 0 for no-success
uint32_t asm_reset_mutex(mutex_t * mutex);  //return 1 for success, 0 for no-success

#endif /*_RTOS_MUTEX_H_*/
