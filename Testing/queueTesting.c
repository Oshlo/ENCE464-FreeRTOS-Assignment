/**********************************
 * queueTesting.c
 *
 * Shows how FreeRTOS Queues are used
 * to pass data between Tasks.
 *     - The LEDQueue stores a the number
 *       of times the LED has turned on
 *       in a uint8_t type.
 *     - The ButtonQueue stores a Struct
 *       with a string containing
 *       information about the button
 *       state and count, and a uint8_t
 *       charLine.
 *     - The OLED display must receive
 *       data from the queue at the same
 *       rate or faster than the queue
 *       is being filled.
 * Is basically a FreeRTOS based version of butsTest.c
 *
 *  Author: Adam Finlayson
 *  Date: 25/07/2021
 *
 **********************************/

/* Standard Includes */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"

/*
 * Important include, allows for use of usnprintf() function.
 * Have to copy ustdlib.c into Project.
 */
#include "utils/ustdlib.h"

/* Libraries for interfacing with OLED Display and Buttons */
#include "OrbitOLED/OrbitOLEDInterface.h"
#include "buttons4.h"

/* FreeRTOS Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* LED definitions */
#define LED_BLINK_RATE      500                                 // (ms) Duration to suspend LED task
#define LED_PIN_RED         1                                   // RED Led pin

/* Button definitions */
#define BUTTON_POLL_RATE    50                                  // About 20Hz

/* Display update definitions */
#define UPDATE_DISPLAY_RATE 50                                  // About 20Hz
#define UPDATE_LED_DISPLAY_RATE 500                           // About 5Hz

/* FreeRTOS definitions */
#define TASK_STACK_DEPTH    32
#define TASK_PRIORITY       4

/*
 * Global Queue definitions.
 * All Tasks can access these Queues.
 */
static xQueueHandle LEDQueue;
static xQueueHandle ButtonQueue;

/* A Struct used by the ButtonQueue */
typedef struct A_Message
{
    char string[17]; // Shows what string to display on the OLED
    uint8_t charLine; // Shows what line to display the string on
} AMessage;

/* Queue Length and Item Sizes used by LEDQueue and ButtonQueue */
#define QUEUE_LENGTH 5
#define LED_QUEUE_ITEM_SIZE sizeof(uint8_t)
#define BUTTON_QUEUE_ITEM_SIZE sizeof(AMessage)


/* Initializes the Clock */
void
initCLK(void)
{
    SysCtlClockSet (SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);  // Set the clock rate to 80 MHz

}


/* Initializes the LED */
void
initGPIO(void)
{
    // For LED blinky task - initialize GPIO port F and then pin #1 (red) for output
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);                // activate internal bus clocking for GPIO port F
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));        // busy-wait until GPIOF's bus clock is ready

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);         // PF_1 as output
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);    // doesn't need too much drive strength as the RGB LEDs on the TM4C123 launchpad are switched via N-type transistors
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);               // off by default
}


/* Initializes the Display */
void
initDisplay (void)
{
    // Initialise the Orbit OLED display
    OLEDInitialise ();

    // Show the Title
    OLEDStringDraw("queueTesting.c", 0, 0);

    // Display fits 16 characters wide. Initialize two character strings
    char string1[17];
    char string2[17];

    // Usually we would use the line below to "Undraw" whatever is currently written on a particular OLED display line,
    // but this seem to take a long time and messes up the display.
    // The code still works because the new strings perfectly overlap the whatever was being displayed before.
    // OLEDStringDraw ("                ", 0, charLine);

    // usnprintf() is the most reliable function for printing to the OLED
    usnprintf (string1, sizeof(string1), "%s - %s %d", "UP  ", "RELS", 0);
    usnprintf (string2, sizeof(string2), "%s - %s %d", "DOWN", "RELS", 0);

    // Draw the two strings to the OLED
    OLEDStringDraw (string1, 0, 2);
    OLEDStringDraw (string2, 0, 3);
}


/* Initializes the Queues */
void
initQueues(void)
{
    LEDQueue = xQueueCreate(QUEUE_LENGTH, LED_QUEUE_ITEM_SIZE); // Create the Global LEDQueue
    if (LEDQueue == NULL) {
        while(1); // No Memory for Queue
    }

    ButtonQueue = xQueueCreate(QUEUE_LENGTH, BUTTON_QUEUE_ITEM_SIZE); // Create the ButtonQueue
    if (ButtonQueue == NULL) {
        while(1); // No Memory for Queue
    }
}


/*
 * A FreeRTOS task that toggles at a rate defined by LED_BLINK_RATE.
 * Sends the LED count data to the LEDQueue at half the toggle rate.
 */
void
BlinkLED(void *pvParameters)
{
    uint8_t whichLed = *((uint8_t *)pvParameters);              // pvParameters is a pointer to an unsigned 8 bit integer - the LED pin number

    const uint8_t whichBit = 1 << whichLed;                     // TivaWare GPIO calls require the pin# as a binary bitmask, not a simple number.
                                                                // Alternately, we could have passed the bitmask into pvParameters instead of a simple number.
    uint8_t currentValue = 0;

    static uint8_t count = 0;   // Initialize count data type
    static uint8_t odd = 0;     // Because the task uses an XOR to toggle the LED, use odd to increment count only when the LED turns on

    while (1)
    {
        odd++; // Increment odd

        currentValue ^= whichBit;                               // XOR keeps flipping the bit on / off alternately each time this runs.
        GPIOPinWrite(GPIO_PORTF_BASE, whichBit, currentValue);
                     // Suspend this task (so others may run) for BLINK_RATE (or as close as we can get with the current RTOS tick setting).

        if (odd % 2 == 1) { // Check if odd is an odd number
            count++; // Increment count
            if (pdTRUE != xQueueSend(LEDQueue, (void*)&count, 10)) { // Check if count data can be sent to the LEDQueue, wait 10 ticks
                while(1); // Cant sent to Queue
            }
        }

        vTaskDelay(LED_BLINK_RATE / portTICK_RATE_MS); // Allows other tasks to be run during this time. Toggles the LED at LED_BLINK_RATE.
    }

    // No way to kill this blinky task unless another task has an xTaskHandle reference to it and can use vTaskDelete() to purge it.
}


/* A Task that  */
void
PollButtons(void* pvParameters)
{
    AMessage ButtonMessage; // Initialize AMessage sturct to be sent to the ButtonQueue

    uint8_t upPushes = 0;  // Initialize count for number of pushes of the up button
    uint8_t downPushes = 0;  // Initialize count for number of pushes of the down button

    while(1)
    {
        // Button Polling Code
        uint8_t butState;

        updateButtons ();       // Poll the buttons

        // check state of each button and send to the ButtonQueue if a change is detected
        butState = checkButton (UP);
        switch (butState)
        {
        case PUSHED:
            usnprintf (ButtonMessage.string, sizeof(ButtonMessage.string), "%s - %s %d", "UP  ", "PUSH", ++upPushes);
            ButtonMessage.charLine = 2;
            break;
        case RELEASED:
            usnprintf (ButtonMessage.string, sizeof(ButtonMessage.string), "%s - %s %d", "UP  ", "RELS", upPushes);
            ButtonMessage.charLine = 2;
            break;
        // Do nothing if state is NO_CHANGE
        }
        butState = checkButton (DOWN);
        switch (butState)
        {
        case PUSHED:
            usnprintf (ButtonMessage.string, sizeof(ButtonMessage.string), "%s - %s %d", "DOWN", "PUSH", ++downPushes);
            ButtonMessage.charLine = 3;
            break;
        case RELEASED:
            usnprintf (ButtonMessage.string, sizeof(ButtonMessage.string), "%s - %s %d", "DOWN", "RELS", downPushes);
            ButtonMessage.charLine = 3;
            break;
        // Do nothing if state is NO_CHANGE
        }

        // Send the message to the ButtonQueue, wait for 10 ticks for available space.
        if (xQueueSend(ButtonQueue, (void *)&ButtonMessage, 10) != pdPASS)
        {
          while(1); // Could not send to Queue
        }

        vTaskDelay(BUTTON_POLL_RATE / portTICK_RATE_MS); // Run the Task at a rate defined by BUTTON_POLL_RATE.
    }
}


/* A task that takes data from the LEDQueue and displays it on the OLED Display */
void
displayLEDCount(void *pvParameters)
{
    uint8_t count; // Create a local count variable to store the LEDQueue Data

    while(1)
    {
        if (xQueueReceive(LEDQueue, (void *)&count, portMAX_DELAY) == pdTRUE) { // Check if something has been received from the LEDQueue
            char textBuffer[17]; // Initialize a char string buffer

            usnprintf (textBuffer, sizeof(textBuffer), "LED COUNT: %d", count); // Place the desired string into the  buffer
            OLEDStringDraw (textBuffer, 0, 1); // Print the Buffer to the OLED display on line 1 (the second line of the OLED)
        }

        //vTaskDelay(UPDATE_LED_DISPLAY_RATE / portTICK_RATE_MS); // Ensure this task updates faster than the LED blinks
    }
}


/* A task that takes data from the ButtonQueue and displays it on the OLED Display */
void
displayButtonState(void* pvParameters)
{

    AMessage ButtonMessage; // Initialize a local stuct variable to store data from ButtonQueue

    while(1)
    {
        // Receive the AMessage Data from the Queue
        if (xQueueReceive(ButtonQueue, (void *)&ButtonMessage, portMAX_DELAY) == pdPASS) { // Check if AMessage Data has been received the ButtonQueue
            OLEDStringDraw (ButtonMessage.string, 0, ButtonMessage.charLine); // Draw the String data to a particular line
        }

        //vTaskDelay(UPDATE_DISPLAY_RATE / portTICK_RATE_MS);  // Update the display faster than the button poll rate
    }
}


/*
 * Initialize the Tasks.
 * Must come just before main() to have access to other task handers.
 */
void
initTasks(void)
{
    static uint8_t led = LED_PIN_RED;                           // LED pin number - static preserves the value while the task is running

    if (pdTRUE != xTaskCreate(BlinkLED, "Blinker", TASK_STACK_DEPTH, (void *) &led, TASK_PRIORITY, NULL)) { // Create the BlinkLED task
        while(1);               // Oh no! Must not have had enough memory to create the task.
    }

    if (pdTRUE != xTaskCreate(PollButtons, "Button Check", TASK_STACK_DEPTH, NULL, TASK_PRIORITY, NULL)) { // Create the PollButtons task
        while(1);
    }

    if (pdTRUE != xTaskCreate(displayLEDCount, "Display LED Count", TASK_STACK_DEPTH, NULL, TASK_PRIORITY, NULL)) { // Create the displayLEDCount task
        while(1);
    }

    if (pdTRUE != xTaskCreate(displayButtonState, "Display Button State", TASK_STACK_DEPTH, NULL, TASK_PRIORITY, NULL)) { // Create the displayButtonState task
        while(1);
    }
}


/* The main function */
int main(void)
{
    initCLK(); // Initialize the Clock
    initGPIO(); // Initialize the LED
    initButtons(); // Initialize the Buttons
    initDisplay(); // Initialize the OLED Display
    initQueues(); // Initialize the Queues
    initTasks(); // Initialize the Tasks

    vTaskStartScheduler();      // Start FreeRTOS!!

    while(1);                   // Should never get here since the RTOS should never "exit".
}

