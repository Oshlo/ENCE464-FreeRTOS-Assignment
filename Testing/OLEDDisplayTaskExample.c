/**
 * A simple file that shows how to use the OLEDDisplayTask with other tasks.
 * Important part are outlined with *********************
 */

// Standard includes
#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/********************************************/
// Includes needed to use the OLEDDisplayTask
#include "OLEDDisplayTask.h"
#include "queueStructs.c"
#include "utils/ustdlib.h"
#include "OrbitOLED/OrbitOLEDInterface.h"
/********************************************/

#define LED_BLINK_RATE      1000                                // (ms) Duration to suspend LED task
#define LED_PIN_RED         1                                   // RED Led pin

#define TASK_STACK_DEPTH    32
#define TASK_PRIORITY       4

/********************************************/
// Have to initialize the OLEDQueue as an extern.
extern xQueueHandle OLEDQueue;
/********************************************/

// Blinky function
void BlinkLED(void *pvParameters)
{
    uint8_t whichLed = *((uint8_t *)pvParameters);              // pvParameters is a pointer to an unsigned 8 bit integer - the LED pin number
    const uint8_t whichBit = 1 << whichLed;                     // TivaWare GPIO calls require the pin# as a binary bitmask, not a simple number.                                                             // Alternately, we could have passed the bitmask into pvParameters instead of a simple number
    uint8_t currentValue = 0;

    /********************************************/
    // Structure used to send data the OLEDDisplayTask
    OLEDMessage OLEDMessage; // Initializing
    OLEDMessage.charLine = 1; // Setting the line for the string to be printed to
    /********************************************/

    while (1)
    {
        currentValue ^= whichBit;                               // XOR keeps flipping the bit on / off alternately each time this runs.
        GPIOPinWrite(GPIO_PORTF_BASE, whichBit, currentValue);

        /***********************************************************************************************/
        // Storing a message into the OLEDMessage string buffer.
        usnprintf (OLEDMessage.strBuf, sizeof(OLEDMessage.strBuf), "LED Value: %d     ", currentValue);

        // Send the Message to the OLEDQueue
        if (pdTRUE != xQueueSend(OLEDQueue, (void*)&OLEDMessage, 10)) { // Check if the message can be sent to the LEDQueue, wait 10 ticks
            while(1); // Can't sent to Queue
        }
        /***********************************************************************************************/

        vTaskDelay(LED_BLINK_RATE / portTICK_RATE_MS);              // Suspend this task (so others may run) for BLINK_RATE (or as close as we can get with the current RTOS tick setting).
    }
    // No way to kill this blinky task unless another task has an xTaskHandle reference to it and can use vTaskDelete() to purge it.
}

/* Initializes the Display */
void
initDisplay (void)
{
    // Initialise the Orbit OLED display
    OLEDInitialise ();

    // Show the Title
    OLEDStringDraw("main.c", 0, 0);
}

int main(void)
{
    static uint8_t led = LED_PIN_RED;                           // LED pin number - static preserves the value while the task is running

    SysCtlClockSet (SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);  // Set the clock rate to 80 MHz

    // For LED blinky task - initialize GPIO port F and then pin #1 (red) for output
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);                // activate internal bus clocking for GPIO port F
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));        // busy-wait until GPIOF's bus clock is ready

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);         // PF_1 as output
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);    // doesn't need too much drive strength as the RGB LEDs on the TM4C123 launchpad are switched via N-type transistors
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);               // off by default

    initDisplay();

    if (initOLEDDisplayTask() != 0) {
        while(1);
    }

    if (pdTRUE != xTaskCreate(BlinkLED, "Blinker", TASK_STACK_DEPTH, (void *) &led, TASK_PRIORITY, NULL))
    {
        while(1);               // Oh no! Must not have had enough memory to create the task.
    }

    vTaskStartScheduler();      // Start FreeRTOS!!

    while(1);                   // Should never get here since the RTOS should never "exit".
}
