/*******************************************************
 * get_yaw_task.c
 *
 * A FreeRTOS task that uses the Port B GPIO Interrupt
 * to trigger the Quadrature decoding, and process the
 * yaw (angular position) data of the HeliRig
 *
 *  Created on: 17/08/2021
 *      Author: Group 1
 *******************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"
#include "utils/ustdlib.h"


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "get_yaw_task.h"
#include "helirig_structs.c"

static volatile QuadType QuadData; // Used by an interrupt so needs to be global

/*******************************************************
 * Function: quadIntHandler
 *
 * Initialises the GPIO interrupt handler
 *******************************************************/
 
void
quadIntHandler(void)
{
    // Set Quadrature data
    QuadData.A = (GPIOPinRead (GPIO_PORTB_BASE, GPIO_PIN_0) == GPIO_PIN_0);
    QuadData.B = (GPIOPinRead (GPIO_PORTB_BASE, GPIO_PIN_1) == GPIO_PIN_1);
    QuadData.L = (QuadData.A^QuadData.Bp);
    QuadData.R = (QuadData.B^QuadData.Ap);
    QuadData.sum = QuadData.sum - QuadData.L + QuadData.R;
    QuadData.Ap = QuadData.A;
    QuadData.Bp = QuadData.B;

    // Clear interrupts
    GPIOIntClear(GPIO_PORTB_BASE, GPIO_INT_PIN_0);
    GPIOIntClear(GPIO_PORTB_BASE, GPIO_INT_PIN_1);
}

/*******************************************************
 * Function: initGPIOInt
 *
 * Initialises the GPIO Pins for interrupt
 *******************************************************/
 
void
initGPIOInt (void)
{
    // Port B must be enabled for configuration and use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));  // busy-wait until GPIOB's bus clock is ready

    // Set the pins
    GPIOPinTypeGPIOInput (GPIO_PORTB_BASE, GPIO_PIN_0);
    GPIOPinTypeGPIOInput (GPIO_PORTB_BASE, GPIO_PIN_1);

    // Register the quadIntHandler to interrupts on port B
    GPIOIntRegister(GPIO_PORTB_BASE, quadIntHandler);

    // Set the interrupt to trigger on both rising and falling edge
    GPIOIntTypeSet(GPIO_PORTB_BASE, GPIO_INT_PIN_0, GPIO_BOTH_EDGES);
    GPIOIntTypeSet(GPIO_PORTB_BASE, GPIO_INT_PIN_1, GPIO_BOTH_EDGES);

    // Enable the interrupts
    GPIOIntEnable(GPIO_PORTB_BASE, GPIO_INT_PIN_0);
    GPIOIntEnable(GPIO_PORTB_BASE, GPIO_INT_PIN_1);

}

/*******************************************************
 * Function: getYawTask
 *
 * Processes the change in the quadrature and converts it
 * into change in degrees
 * Writes the current angle to the FreeRTOS Queue OLEDQueue
 *******************************************************/

void getYawTask (void *pvParameters)
{
    // Initialise variables
    xQueueHandle OLEDQueue = *((xQueueHandle *)pvParameters);

    int32_t angle;
    // used for sending a message to the OLED display function
    OLEDMessage OLEDMessage;
    OLEDMessage.charLine = 2;
    OLEDMessage.charPos = 0;

    while (1)
    {
        // Map quadrature to angle and apply limits
        angle = (int32_t)((long) QuadData.sum * 0.8);
        if (angle > 359){
            angle = 0;
            QuadData.sum = -1;
        }
        if (angle < 0){
            angle = 359;
            QuadData.sum = 449;
        }

        // store message for OLED display
        usnprintf(OLEDMessage.strBuf, sizeof(OLEDMessage.strBuf), "Angle (deg): %d     ", angle);

        // Send to OLED queue
        if (xQueueSend(OLEDQueue, (void *)&OLEDMessage, 10) != pdPASS)
        {
            while(1); // Could not send to Queue
        }

        // Delay
        vTaskDelay(YAW_TASK_HZ / portTICK_RATE_MS);
    }
}


/*******************************************************
 * Function: initGetYawTask
 *
 * Creates the FreeRTOS task GetYawTask
 * Initialises the GPOI interrupt pins
 *
 * returns: 0 on successful creation of GetYawTask
 *          1 on failed attempt
 *******************************************************/

uint8_t
initGetYawTask(xQueueHandle* OLEDQueue)
{
    initGPIOInt(); // Initialise GPIO interrupts

    // Preset quad data
    QuadData.Ap = 0;
    QuadData.Bp = 0;
    QuadData.sum = 1;

    // Create getYawTask
    if (pdTRUE != xTaskCreate(getYawTask, "Get Yaw Data", TASK_STACK_DEPTH, (void *) OLEDQueue, TASK_PRIORITY, NULL))
    {
        return(1);               // Oh no! Must not have had enough memory to create the task.
    }

    return(0);

}
