#ifndef __GET_HEIGHT_TASK_H__
#define __GET_HEIGHT_TASK_H__

/*******************************************************
 * get_height_task.c
 *
 * A FreeRTOS task that uses a timer (TIMER1) interrupt the trigger the ADC,
 * saving the contents to a circular buffer
 *
 *  Created on: 12/08/2021
 *      Author: Group 1
 *******************************************************/


/*******************************************************
 * Constants
 *******************************************************/
#define TASK_STACK_DEPTH    32
#define TASK_PRIORITY       4

#define BUF_SIZE 4
#define ADC_DISPLAY_RATE    25  // in ms
#define SAMPLE_RATE_HZ      10

/*******************************************************
 * Function: initADC
 *
 * Initialises the ADC
 *******************************************************/
void initADC (void);


/*******************************************************
 * Function: initTimer
 *
 * Initialises a timer interrupt used to poll the ADC
 *******************************************************/
void
initTimer(void);


/*******************************************************
 * Function: timerIntHandler
 *
 * Periodic timer interrupt used to trigger the ADC
 * interrupt handler.
 * (There is a more efficient way to do this by triggering
 * the ACD interrupt handler directly in TimerIntRegister())
 *******************************************************/
void
timerIntHandler(void);


/*******************************************************
 * Function: ADCIntHandler
 *
 * Triggered by the timer interrupt and writes the output to a circular buffer
 *******************************************************/
void
ADCIntHandler(void);


/*******************************************************
 * Function: GetHeightTask
 *
 * Reads the circular buffer and averages the values to get current height
 * Writes the current height to the FreeRTOS Queue OLEDQueue
 *
 * pvParameters: NULL
 *******************************************************/
void
getHeightTask (void *pvParameters);


/*******************************************************
 * Function: initGetHeightTask
 *
 * Creates the FreeRTOS task GetHeightTask
 *      Initialises the ADC, a timer, and a circular buffer
 *
 * returns: 0 on successful creation of GetHeightTask
 *          1 on failed attempt
 *******************************************************/
uint8_t
initGetHeightTask(xQueueHandle* OLEDQueue);


#endif /* __GET_HEIGHT_TASK_H__ */
