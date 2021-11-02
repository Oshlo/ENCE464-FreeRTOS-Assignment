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


#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/debug.h"
#include "driverlib/interrupt.h"
#include "utils/ustdlib.h"
#include "OrbitOLED/OrbitOLEDInterface.h"


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "OLED_display_task.h"
#include "helirig_structs.c"


/*******************************************************
 * Function: OLEDDisplayTask
 *
 * Reads data from FreeRTOS Queue OLEDQueue and displays it
 * on the designated line of the OLED display
 *******************************************************/
static void
OLEDDisplayTask (void *pvParameters)
{
    // Initalise variable
    xQueueHandle Queue = *((xQueueHandle *) pvParameters);

    OLEDMessage OLEDMessage;

    while(1)
    {
        if (xQueueReceive(Queue, (void *)&OLEDMessage, portMAX_DELAY) == pdTRUE) { // Check if something is available on the queue

            IntMasterDisable();  // Disable all interrupts to protect OLEDStringDraw();

            if (OLEDMessage.charLine != 0) {
                OLEDStringDraw (OLEDMessage.strBuf, OLEDMessage.charPos, OLEDMessage.charLine); // Draw to OLED display
            }

            IntMasterEnable();  // Re-enable interrupts
        }
    }
}


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
initOLEDDisplayTask (xQueueHandle* OLEDQueue)
{
    // Create the Global LEDQueue
    //OLEDQueue = xQueueCreate(OLED_QUEUE_LENGTH, OLED_QUEUE_ITEM_SIZE);
    if (OLEDQueue == NULL) {
        return(2);  // No Memory for Queue
    }

    // Create the OLED Display task
    if (pdTRUE != xTaskCreate(OLEDDisplayTask, "OLED Display Task", TASK_STACK_DEPTH, (void *) OLEDQueue, TASK_PRIORITY, NULL)) {
        return(1);  // Fail (Must not have had enough memory to create the task)
    }

    return(0);  // Success.
}
