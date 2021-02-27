#ifndef _RTOS_MUTEX_H_
#define _RTOS_MUTEX_H_

#include "RTOS_ish.h"

typedef struct {
  
  uint32_t lock_token;
  void * mutex_ptr;
  
}mutex_t;

mutex_t * mutex_create(void);


uint32_t asm_set_mutex(mutex_t * mutex);
uint32_t asm_reset_mutex(mutex_t * mutex);

#endif /*_RTOS_MUTEX_H_*/
