#include "rtos_mutex.h"
#include "RTOS_ish.h"
#include "stm32f4xx.h"

#define LOG2(x) (32 - __clz(x))

extern TaskTCB * volatile OS_currentTask;  //Pointer to current task to execute
extern TaskTCB * volatile OS_nextTask;     //Pointer to next task to execute

extern uint32_t OS_readySet;               //bitmask of tasks ready to run
extern uint32_t OS_delaySet;               //bitmask of tasks delayed by OS_delay

mutex_controller_t * mutex_ctrl_ptr;

void OS_mutexFreeList_create(void)
{
  mutex_ctrl_ptr = (mutex_controller_t *)malloc(sizeof(mutex_controller_t));
  mutex_ctrl_ptr->availableList = 0xFFFFFFFFU;
  for(uint32_t i = 0; i < 32; i++)
  {
    mutex_ctrl_ptr->mutexFreeList[i] = (mutex_t *)malloc(sizeof(mutex_t));
  }
}

mutex_t * OS_mutex_create(void)
{ 
  mutex_t * temp_ptr;
  uint8_t availableIdx;
  
  if(mutex_ctrl_ptr->availableList != 0x00000000U)
  {
    __asm volatile(
      "EOR    r1, %[availableList], #0xFFFFFFFFU \n\t"
      "ADD    r0, r1, #0x01\n\t"
      "AND    r1, %[availableList], r0\n\t"
      "CLZ    r0, r1              \n\t"
      "MOV    r1, #0x1FU\n\t"
      "SUB    %[availableIdx],  r1, r0\n\t"
      :[availableIdx] "=r" (availableIdx)
      :[availableList] "r" (mutex_ctrl_ptr->availableList)
      :"memory", "r0", "r1"
    );   
  mutex_ctrl_ptr->availableList &= ~(1 << availableIdx);
  temp_ptr = mutex_ctrl_ptr->mutexFreeList[availableIdx];
  temp_ptr->FreeListIdx = (1 << availableIdx);
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
  mutex_ctrl_ptr->availableList |= (temp_ptr->FreeListIdx);
  (temp_ptr->FreeListIdx) = 0x00000000U;
  *mutex = NULL;
}
/*non-blocking mutex apis*/
uint8_t OS_nonblocking_mutex_lock(mutex_t * mutex)
{
  uint8_t isLockSuccessful;
  isLockSuccessful = asm_set_mutex(mutex);
  __disable_irq();
  if(isLockSuccessful)
  {
    mutex->OwnerTask = OS_currentTask;
  }
  __enable_irq();
  return isLockSuccessful;
}

uint8_t OS_nonblocking_mutex_unlock(mutex_t * mutex)
{
  uint8_t isUnlockSuccssful;
  isUnlockSuccssful = asm_reset_mutex(mutex);
  __disable_irq();
  if(isUnlockSuccssful)
  {
    mutex->OwnerTask = (void *)0x00U;
  }
  __enable_irq();
  return isUnlockSuccssful;
}

//doesnt work, dont use
void OS_blocking_mutex_lock(mutex_t * mutex)
{
  uint8_t isLockSuccessful;
  isLockSuccessful = asm_set_mutex(mutex);
  if(isLockSuccessful == 0)
  {
    __disable_irq();
    mutex->WaitingList |= (1 << (OS_currentTask->workingPriority - 1U));
    OS_readySet &= ~(1 << (OS_currentTask->workingPriority - 1U));
    if(OS_currentTask->workingPriority <= mutex->OwnerTask->workingPriority)
    {
      OS_currentTask->workingPriority = ((mutex->OwnerTask->workingPriority) + 1U);
    }
    OS_delaySet |=  (1 << (OS_currentTask->workingPriority - 1U));
    OS_Schedule();
    __enable_irq();
  }
}
 
//doesnt work, dont use
void OS_blocking_mutex_unlock(mutex_t * mutex)
{
  uint8_t isUnlockSuccessful;
  isUnlockSuccessful = asm_reset_mutex(mutex);
  if(isUnlockSuccessful)
  {
    __disable_irq();
    OS_readySet |= (1 << (mutex->WaitingList - 1U));
    OS_currentTask->workingPriority = OS_currentTask->setPriority;
    __enable_irq();
  }
}

uint32_t asm_set_mutex(mutex_t * mutex)
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

uint32_t asm_reset_mutex(mutex_t * mutex)
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
