/*******************************************************
 * main.c
 *
 * The primary run file for the HeliRig Project.
 *
 *  Created on: 12/08/2021
 *      Author: Group 1
 *******************************************************/

// Standard Library
#include <stdint.h>
#include <stdbool.h>

// Macros
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"

// Drivers
#include "driverlib/adc.h"
#include "driverlib/pwm.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/pin_map.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"

// Orbit OLED Display
#include "OrbitOLED/OrbitOLEDInterface.h"

// Utilities
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// Tasks
#include "get_height_task.h"
#include "get_yaw_task.h"
#include "OLED_display_task.h"
#include "helirig_structs.c"




/*******************************************************
 * Function: initCLK
 *
 * Initialises the clock to 80 MHz
 *******************************************************/
void
initCLK(void)
{
    SysCtlClockSet (SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);  // Set the clock rate to 80 MHz
}


/*******************************************************
 * Function: initDisplay
 *
 * Initialises the OLED Display
 *      Draws "HeliRig Project"
 *******************************************************/
void
initDisplay (void)
{
    OLEDInitialise ();
    OLEDStringDraw("HeliRig Project", 0, 0);
    OLEDStringDraw("EXP: 5, 10", 0, 3);
}


/*******************************************************
 * Function: main
 *
 * The main loop
 *
 * returns: Does not return so long as FreeRTOS is running
 *******************************************************/
uint32_t
main(void)
{
    initCLK();  // Initialise the Clock
    initDisplay();  // Initialise the Display

    // Random Test
    xQueueHandle OLEDQueue = xQueueCreate(OLED_QUEUE_LENGTH, OLED_QUEUE_ITEM_SIZE);
    if (OLEDQueue == NULL) {
        return(2);  // No Memory for Queue
    }

    if(initGetHeightTask(&OLEDQueue) != 0) {while(1);}

    if(initGetYawTask(&OLEDQueue) != 0) {while(1);}

    if(initOLEDDisplayTask(&OLEDQueue) != 0) {while(1);}

    IntMasterEnable();  // Enable interrupts

    vTaskStartScheduler();  // Start FreeRTOS

    while(1);  // Should never get here since the RTOS should never "exit".
}

