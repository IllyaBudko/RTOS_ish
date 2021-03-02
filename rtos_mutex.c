#include "RTOS_ish.h"
#include "rtos_mutex.h"

mutex_controller_t * mutex_ctrl_ptr;

mutex_t * OS_mutex_create(void)
{ 
  mutex_t * temp_ptr;
  uint8_t availableIdx;
  
  if(mutex_ctrl_ptr->availableList != 0x00000000U)
  {
    __asm volatile(
      "MOV    r0, #0xFFFFFFFEU  \n\t"
      "EOR    r1, %[availableList], r0 \n\t"
      "CLZ    %[availableIdx], r1              \n\t"
      :[availableIdx] "=r" (availableIdx)
      :[availableList] "r" (mutex_ctrl_ptr->availableList)
      :"memory", "r0", "r1"
    );
  temp_ptr = mutex_ctrl_ptr->mutexFreeList[availableIdx];
  mutex_ctrl_ptr->availableList &= ~((1 << (31 - availableIdx)));
  temp_ptr->mutexFreeListIdx |= ((1 << (31 - availableIdx)));
  }
  else
  {
    temp_ptr = (mutex_t *)0;
    return temp_ptr;
  }
  
  return temp_ptr;
}

void OS_mutex_destroy(mutex_t ** mutex)
{
  mutex_t *temp_ptr = *mutex;
  mutex_ctrl_ptr->availableList &= ~(1 << ((temp_ptr->mutexFreeListIdx) - 1U));
  (temp_ptr->mutexFreeListIdx) = 0x00000000U;
  *mutex = NULL;
}

void OS_mutexFreeList_create(void)
{
  mutex_ctrl_ptr = (mutex_controller_t *)malloc(sizeof(mutex_controller_t));
  mutex_ctrl_ptr->availableList = 0xFFFFFFFFU;
  for(uint32_t i = 0; i < 31; i++)
  {
    mutex_ctrl_ptr->mutexFreeList[i] = (mutex_t *)malloc(sizeof(mutex_t));
  }
  
}

uint32_t OS_set_mutex(mutex_t * mutex)
{
  uint32_t set_success = 0x01;
  if((void *)mutex != NULL)
  {
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
  }
  return set_success == 0;
}

uint32_t OS_reset_mutex(mutex_t * mutex)
{
  uint32_t set_success = 0x01;
  if((void *)mutex != NULL)
  {
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
  }
  return set_success == 0;
}
