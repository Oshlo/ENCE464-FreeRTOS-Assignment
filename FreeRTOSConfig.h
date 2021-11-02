/*
 * FreeRTOSConfig.h
 *
 *  Created on: 15/07/2020
 *      Author: andre
 */

#ifndef FREERTOSCONFIG_H_
#define FREERTOSCONFIG_H_


/* Per-Project FreeRTOS Configuration */

/*
 * Check all the required application specific macros have been defined.
 * These macros are application specific and (as downloaded) are defined
 * within FreeRTOSConfig.h.
 */

#define configMINIMAL_STACK_SIZE 128 // Idle task stack (in 32-bit words)

#define configMAX_PRIORITIES 8 // Adjustable but I kept it at 8 for kicks.

#define configUSE_PREEMPTION 1

#define configUSE_IDLE_HOOK 0

#define configUSE_TICK_HOOK 0

#define INCLUDE_vTaskPrioritySet 0

#define INCLUDE_uxTaskPriorityGet 0

#define INCLUDE_vTaskDelete 1

#define INCLUDE_vTaskSuspend 1

#define INCLUDE_vTaskDelayUntil 0

#define INCLUDE_vTaskDelay 1

#define configUSE_16_BIT_TICKS 0 // not sure what this is

#define configKERNEL_INTERRUPT_PRIORITY (7 << 5) // Lowest priority for RTOS periodic interrupts

#define configMAX_SYSCALL_INTERRUPT_PRIORITY (1 << 5) // Leaves IRQ priority 0 for any non-RTOS Real Time interrupts

#define configTOTAL_HEAP_SIZE (8 * 1024) // Adjustable - TM4C123 should support at least 24KB heap

#define configCPU_CLOCK_HZ 80000000UL // Full 80MHz clock

#define configTICK_RATE_HZ 1000 // 1ms SysTick ticker


#endif /* FREERTOSCONFIG_H_ */
