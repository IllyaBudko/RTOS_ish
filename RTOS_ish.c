

#include "RTOS_ish.h"
#include "stm32f4xx.h"





void OSTask_Create(TaskTCB *me,OSTaskHandler Task, size_t stkSize)
{
  me->sp = (uint32_t *)calloc(stkSize, sizeof(uint32_t));
  
  uint32_t * temp_ptr = (uint32_t *)(me->sp);
  uint32_t * temp_ptr_limit = (uint32_t *)((uint32_t)temp_ptr + (sizeof(uint32_t) * stkSize));
  
  for(temp_ptr = temp_ptr; temp_ptr < temp_ptr_limit ; temp_ptr++)
  {
    *temp_ptr = 0xDEADBEEFU;
  }
}






















