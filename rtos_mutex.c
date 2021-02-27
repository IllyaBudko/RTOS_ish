#include "RTOS_ish.h"
#include "rtos_mutex.h"

mutex_t * mutex_create(uint32_t init_state)
{
  mutex_t * mutex_ptr;
  mutex_ptr->mutex_ptr = (uint32_t *)malloc(sizeof(mutex_t));
  
  mutex_ptr->lock_token = init_state;
  
  return mutex_ptr;
}

uint32_t asm_set_mutex(mutex_t * mutex)
{
  uint32_t set_success = 0x01;
  __asm(
  
  "MOV      r0            , #0x01  \n\t"
  "LDREX    %[set_success], [%[mutex]]  \n\t"
  "CMP      %[set_success], #0x00 \n\t"
  "ITT      EQ  \n\t"
  "STREXEQ  %[set_success], r0, [%[mutex]]  \n\t"
  "DMBEQ  \n\t"
  :[set_success] "=r" (set_success), [mutex] "+r" (mutex)
  :
  :"memory", "r0"
  );
  return set_success == 0;
}

uint32_t asm_reset_mutex(mutex_t * mutex)
{
  uint32_t set_success = 0x01;
  __asm(
  
  "MOV      r0            , #0x00  \n\t"
  "LDREX    %[set_success], [%[mutex]]  \n\t"
  "CMP      %[set_success], #0x01 \n\t"
  "ITT      EQ  \n\t"
  "STREXEQ  %[set_success], r0, [%[mutex]]  \n\t"
  "DMBEQ  \n\t"
  :[set_success] "=r" (set_success), [mutex] "+r" (mutex)
  :
  :"memory", "r0"
  );
  return set_success == 0;
}
