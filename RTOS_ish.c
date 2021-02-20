

#include "RTOS_ish.h"
#include "stm32f4xx.h"





void OSTask_Create(TaskTCB *me,OSTaskHandler Task, size_t stkSize)
{
  me->stk_alloc = (uint32_t *)calloc(stkSize, sizeof(uint32_t));
  //me->sp = (uint32_t *)calloc(stkSize, sizeof(uint32_t));
  
  //uint32_t * temp_ptr = (uint32_t *)(me->sp);
  //uint32_t * temp_ptr_limit = (uint32_t *)((uint32_t)temp_ptr + (sizeof(uint32_t) * (stkSize - 1)));
  
  uint32_t * temp_sp = (uint32_t *)((uint32_t)(me->stk_alloc) + (sizeof(uint32_t) * stkSize - 1));
  temp_sp = (uint32_t *)(((uint32_t)temp_sp / 8) * 8);
  
  *(temp_sp--) = (1 << 24);   //xPSR
  *(temp_sp--) = 
  
  
  for(temp_ptr = temp_ptr; temp_ptr <= temp_ptr_limit ; temp_ptr++)
  {
    *temp_ptr = 0xDEADBEEFU;
  }
}






















