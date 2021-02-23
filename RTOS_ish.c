
#include "main.h"
#include "RTOS_ish.h"
#include "stm32f4xx.h"

#include "qassert.h"

Q_DEFINE_THIS_FILE

#define systick_Pin GPIO_PIN_6
#define systick_GPIO_Port GPIOB

TaskTCB * volatile OS_currentTask;  //Pointer to current task to execute
TaskTCB * volatile OS_nextTask;     //Pointer to next task to execute

TaskTCB * OS_Tasks[32 + 1];   //Task list

uint32_t OS_readySet;               //bitmask of tasks ready to run
uint32_t OS_delaySet;               //bitmask of tasks delayed by OS_delay

TaskTCB MainIdleTask;               //TCB for main idle task

#define LOG2(x) (32 - __clz(x))

void IdleTask_func(void)
{
  while(1)
  {
    OS_OnIdle();
  }
}

void OS_Init(size_t stkSize)
{
  //Set PendSV priority as lowest and Systick just above it
  *(uint32_t volatile *) 0xE000ED20U = 0xEEFF0000U;
  
  //Start idle thread
  OSTask_Create(&MainIdleTask,IdleTask_func,0,stkSize);
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
  if(OS_readySet == 0)
  {
    OS_nextTask = OS_Tasks[0];
  }
  else
  {
    OS_nextTask = OS_Tasks[LOG2(OS_readySet)];
    if(OS_nextTask == (TaskTCB *)0)
    {
      Error_Handler();
    }
  }
  if(OS_nextTask != OS_currentTask)
  {
    *(uint32_t volatile *)0xE000ED04 |= (1 << 28); // Set PendSV Pending
  }
}

void OS_Tick(void)
{
  uint32_t workingSet = OS_delaySet;
  while(workingSet != 0U)
  {
    TaskTCB * t = OS_Tasks[LOG2(workingSet)];
    uint32_t bit = 0;
    
    Q_ASSERT((t != (TaskTCB *)0) && (t->timeout != 0U));
    
    bit = (1 << (t->priority - 1U));
    --t->timeout;
    if(t->timeout == 0U)
    {
      OS_readySet |= bit;
      OS_delaySet &= ~bit;
    }
    workingSet &= ~bit;
  }
}
void OS_Delay(uint32_t ticks)
{
  uint32_t bit = 0;
  __disable_irq();
  
  if(OS_currentTask != OS_Tasks[0])
  {
    OS_currentTask->timeout = ticks;
    bit = (1 << (OS_currentTask->priority - 1U));
    OS_readySet &= ~bit;
    OS_delaySet |=  bit;
    OS_Schedule();
  }
  __enable_irq();
}

void OSTask_Create(TaskTCB *me,OSTaskHandler Task,uint8_t me_priority, size_t stkSize)
{
  uint32_t * temp_sp;
  uint32_t * temp_stack_limit;
  
  //Allocate private stack space
  me->stk_alloc = (uint32_t *)calloc(stkSize, sizeof(uint32_t));
  
  //Creating temporary pointer at the back of allocated memory for private task SP
  temp_sp = (uint32_t *)((uint32_t)(me->stk_alloc) + (sizeof(uint32_t) * stkSize));
  
  //Aligning temporary SP to nearest 8byte alignment within allocated memory
  temp_sp = (uint32_t *)(((uint32_t)temp_sp / 8) * 8);
  
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
  
  //Create pointer to stack limit
  temp_stack_limit = me->stk_alloc;
  
  //Aligning stack limit pointer to nearest 8 byte boundary within allocated memory
  temp_stack_limit = (uint32_t *)(((((uint32_t)temp_stack_limit - 1) / 8) + 1)* 8);

  //Fill free space in private stack with recognizable pattern (0xDEADBEEF)
  for(temp_sp = temp_sp - 1; temp_sp >= temp_stack_limit ; temp_sp--)
  {
    *temp_sp = 0xDEADBEEFU;
  }
  
  if((me_priority < 32) && (OS_Tasks[me_priority] == (TaskTCB *)0))
  {
    OS_Tasks[me_priority] = me;
    me->priority = me_priority;
    if(me_priority > 0U)
    {
      OS_readySet |= (1U << (me_priority - 1U));
    }
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
  OS_Tick();
  OS_Schedule();
  __enable_irq();
}

void Error_Handler(void)
{
  while(1);
}















