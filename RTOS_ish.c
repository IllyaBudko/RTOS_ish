

#include "RTOS_ish.h"
#include "stm32f4xx.h"





void OSTask_Create(TaskTCB *me,OSTaskHandler Task, size_t stkSize)
{
  uint32_t * temp_sp;
  uint32_t * temp_stack_limit;
  
  me->stk_alloc = (uint32_t *)calloc(stkSize, sizeof(uint32_t));
  //me->sp = (uint32_t *)calloc(stkSize, sizeof(uint32_t));
  
  //uint32_t * temp_ptr = (uint32_t *)(me->sp);
  
  temp_sp = (uint32_t *)((uint32_t)(me->stk_alloc) + (sizeof(uint32_t) * stkSize - 1)); //Creating temporary pointer at the back of allocated memory for private task SP
  temp_sp = (uint32_t *)(((uint32_t)temp_sp / 8) * 8);  //Aligning temporary SP to nearest 8byte alignment within allocated memory
  
  *(temp_sp--) = (1 << 24);       //xPSR
  *(temp_sp--) = (uint32_t)Task;  //PC, R15
  *(temp_sp--) = 0x0E1E2E3EU;     //LR, R14
  *(temp_sp--) = 0x0D1D2D3DU;     //    R12
  *(temp_sp--) = 0x03132333U;     //    R03
  *(temp_sp--) = 0x02122232U;     //    R02
  *(temp_sp--) = 0x01112131U;     //    R01
  *(temp_sp--) = 0x00102030U;     //    R00
  
  *(temp_sp--) = 0x0C1C2C3CU;     //    R11
  *(temp_sp--) = 0x0B1B2B3BU;     //    R10
  *(temp_sp--) = 0x0A1A2A3AU;     //    R09
  *(temp_sp--) = 0x08182838U;     //    R08
  *(temp_sp--) = 0x07172737U;     //    R07
  *(temp_sp--) = 0x06162636U;     //    R06
  *(temp_sp--) = 0x05152535U;     //    R05
  *(temp_sp--) = 0x04142434U;     //    R04
  
  me->sp = temp_sp;
  
  //temp_ptr_limit = (uint32_t *)(((((uint32_t)(me->stk_alloc) - 1) / 8) + 1) * 8);
  temp_stack_limit = me->stk_alloc; //Create pointer to stack limit
  temp_stack_limit = (uint32_t *)(((((uint32_t)temp_stack_limit - 1) / 8) + 1)* 8); //Aligning stack limit pointer to nearest 8 byte boundary within allocated memory

  for(temp_sp = temp_sp; temp_sp >= temp_stack_limit ; temp_sp--)
  {
    *temp_sp = 0xDEADBEEFU;
  }
}






















