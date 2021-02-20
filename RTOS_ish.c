
#include "main.h"
#include "RTOS_ish.h"
#include "stm32f4xx.h"


#define systick_Pin GPIO_PIN_6
#define systick_GPIO_Port GPIOB

#define MAX_TASK_NUM    32 + 1

TaskTCB * volatile OS_currentTask;  //Pointer to current task to execute
TaskTCB * volatile OS_nextTask;     //Pointer to next task to execute

TaskTCB * OS_Tasks[MAX_TASK_NUM];            //Task list
uint8_t OS_TaskNum;                 //Number of tasks started
uint8_t OS_CurrentIdx;              //Current task index

void OS_Init(void)
{
  *(uint32_t volatile *) 0xE000ED20U = 0xEEFF0000U; //Set PendSV priority as lowest and Systick just above it
}

void OS_Run(void)
{
  __disable_irq();
  OS_OnStartUp();
  OS_Schedule();
  __enable_irq();
  
  //never will return after enable_irq
}

void OS_Schedule(void)
{
  ++OS_CurrentIdx;
  if(OS_CurrentIdx == OS_TaskNum)
  {
    OS_CurrentIdx = 0U;
  }
  OS_nextTask = OS_Tasks[OS_CurrentIdx];
  if(OS_nextTask != OS_currentTask)
  {
    *(uint32_t volatile *)0xE000ED04 |= (1 << 28); // Set PendSV Pending
  }
}

void OSTask_Create(TaskTCB *me,OSTaskHandler Task, size_t stkSize)
{
  uint32_t * temp_sp;
  uint32_t * temp_stack_limit;
  
  me->stk_alloc = (uint32_t *)calloc(stkSize, sizeof(uint32_t));
  //me->sp = (uint32_t *)calloc(stkSize, sizeof(uint32_t));
  
  //uint32_t * temp_ptr = (uint32_t *)(me->sp);
  
  temp_sp = (uint32_t *)((uint32_t)(me->stk_alloc) + (sizeof(uint32_t) * stkSize)); //Creating temporary pointer at the back of allocated memory for private task SP
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
  * temp_sp    = 0x04142434U;     //    R04
  
  me->sp = temp_sp;
  
  //temp_ptr_limit = (uint32_t *)(((((uint32_t)(me->stk_alloc) - 1) / 8) + 1) * 8);
  temp_stack_limit = me->stk_alloc; //Create pointer to stack limit
  temp_stack_limit = (uint32_t *)(((((uint32_t)temp_stack_limit - 1) / 8) + 1)* 8); //Aligning stack limit pointer to nearest 8 byte boundary within allocated memory

  for(temp_sp = temp_sp - 1; temp_sp >= temp_stack_limit ; temp_sp--)
  {
    *temp_sp = 0xDEADBEEFU;
  }
  if(OS_TaskNum < MAX_TASK_NUM)
  {
    OS_Tasks[OS_TaskNum] = me;
    ++OS_TaskNum;
  }
  else
  {
    Error_Handler();
  }
    
}

__asm
void PendSV_Handler(void)
{

    IMPORT  OS_currentTask  /* extern variable */
    IMPORT  OS_nextTask  /* extern variable */

    /* __disable_irq(); */
    CPSID         I

    /* if (OS_curr != (OSThread *)0) { */
    LDR           r1,=OS_currentTask
    LDR           r1,[r1,#0x00]
    CBZ           r1,PendSV_restore

    /*     push registers r4-r11 on the stack */
    PUSH          {r4-r11}

    /*     OS_curr->sp = sp; */
    LDR           r1,=OS_currentTask
    LDR           r1,[r1,#0x00]
    STR           sp,[r1,#0x00]
    /* } */

PendSV_restore
    /* sp = OS_next->sp; */
    LDR           r1,=OS_nextTask
    LDR           r1,[r1,#0x00]
    LDR           sp,[r1,#0x00]

    /* OS_curr = OS_next; */
    LDR           r1,=OS_nextTask
    LDR           r1,[r1,#0x00]
    LDR           r2,=OS_currentTask
    STR           r1,[r2,#0x00]

    /* pop registers r4-r11 */
    POP           {r4-r11}

    /* __enable_irq(); */
    CPSIE         I

    /* return to the next thread */
    BX            lr
}

void SysTick_Handler(void)
{
  __disable_irq();
  HAL_GPIO_TogglePin(systick_GPIO_Port,systick_Pin);
  HAL_GPIO_TogglePin(systick_GPIO_Port,systick_Pin);
  OS_Schedule();
  __enable_irq();
}

void Error_Handler(void)
{
  while(1);
}















