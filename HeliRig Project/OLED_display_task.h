#ifndef __OLED_DISPLAY_TASK_H__
#define __OLED_DISPLAY_TASK_H__

/*******************************************************
 * OLEDDisplayTask.h
 *
 * A FreeRTOS task that reads data from the a queue and display it on the Orbit OLED display.
 *
 * The key data displayed is the current and expected height, the current and expected yaw, and possibly the mode/state.
 * The queue used by the OLEDDisplayTask contains a 17 char long string variable, and a uint8_t character line variable.
 *
 * To interface with the OLED Display, initialise the OLEDQueue as a extern global variable then send the desired string and
 * character line to the queue.
 *
 * For an Example on how to use the OLEDDisplayTask, see OLEDDisplayTaskExample.c in the Testing Folder.
 *
 *  Created on: 12/08/2021
 *      Author: Group 1
 *******************************************************/


#define TASK_STACK_DEPTH    32
#define TASK_PRIORITY       4

#define OLED_QUEUE_LENGTH 5
#define OLED_QUEUE_ITEM_SIZE sizeof(OLEDMessage)


/*******************************************************
 * Function: OLEDDisplayTask
 *
 * Reads data from FreeRTOS Queue OLEDQueue and displays it
 * on the designated line of the OLED display
 *******************************************************/
static void
OLEDDisplayTask (void *pvParameters);


/*******************************************************
 * Function: initOLEDDisplayTask
 *
 * Creates the FreeRTOS task OLEDDisplayTask
 * Creates the FreeRTOS queue LEDQueue
 *
 * returns: 0 on successful creation of GetHeightTask
 *          1 on failed attempt to create task
 *          2 on failed attempt to create queue
 *******************************************************/
uint8_t
initOLEDDisplayTask (xQueueHandle * xQueue);


#endif /* __OLED_DISPLAY_TASK_H__ */
