#ifndef __GET_YAW_TASK__
#define __GET_YAW_TASK__

/*******************************************************
 * get_yaw_task.c
 *
 * A FreeRTOS task that uses the Port B GPIO Interrupt
 * to trigger the Quadrature decoding, and process the
 * yaw (angular position) data of the HeliRig
 *
 *  Created on: 12/08/2021
 *      Author: Group 1
 *******************************************************/
 
 /*******************************************************
 * Constants
 *******************************************************/

#define TASK_STACK_DEPTH    32
#define TASK_PRIORITY       4

#define YAW_TASK_HZ         100

#define QUEUE_LENGTH 5
#define QUAD_QUEUE_ITEM_SIZE sizeof(int8_t)


/*******************************************************
 * Function: quadIntHandler
 *
 * Initialises the GPIO interrupt handler
 *******************************************************/
 
void quadIntHandler(void);

/*******************************************************
 * Function: initGPIOInt
 *
 * Initialises the GPIO Pins for interrupt
 *******************************************************/

void initGPIOInt(void);

/*******************************************************
 * Function: getYawTask
 *
 * Processes the change in the quadrature and converts it
 * into change in degrees
 * Writes the current angle to the FreeRTOS Queue OLEDQueue
 *******************************************************/

void getYawTask (void *pvParameters);

/*******************************************************
 * Function: initGetYawTask
 *
 * Creates the FreeRTOS task GetYawTask
 * Initialises the GPOI interrupt pins
 *
 * returns: 0 on successful creation of GetYawTask
 *          1 on failed attempt
 *******************************************************/
 
uint8_t initGetYawTask(xQueueHandle* OLEDQueue);

#endif /* __GET_YAW_TASK__ */

