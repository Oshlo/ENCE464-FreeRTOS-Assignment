/*******************************************************
 * get_height_task.c
 *
 * A FreeRTOS task that uses a timer (TIMER1) interrupt the trigger the ADC,
 * saving the contents to a circular buffer
 *
 *  Created on: 12/08/2021
 *      Author: Group 1
 *******************************************************/


#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"

#include "driverlib/adc.h"
#include "driverlib/pwm.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/pin_map.h"
#include "driverlib/timer.h"

#include "utils/ustdlib.h"

#include "circBufT.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "get_height_task.h"
#include "helirig_structs.c"


// From what I can tell this needs to be global as it is being accessed by the ADC Interrupt Handler
static circBuf_t g_inBuffer;  // Buffer of size BUF_SIZE integers (sample values)


/*******************************************************
 * Function: initADC
 *
 * Initialises the ADC
 *******************************************************/
void
initADC (void)
{
    // Enable ADC0 peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0));  // busy-wait until ADC0's bus clock is ready

    // Configure the ADC to process a single sample (sequence 3) when the processor triggers
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    // Configure ADC Sequence 3
    //      Sample channel 9 (ADC_CTL_CH9) for ADC from emulator height output.
    //      Set the interrupt flag to be set after the data sample has been processed (ADC_CTL_IE).
    //      End of sequence (ADC_CTL_END)
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);

    // Enable ADC sequence 3
    ADCSequenceEnable(ADC0_BASE, 3);

    // Register the interrupt handler
    ADCIntRegister(ADC0_BASE, 3, ADCIntHandler);

    // Enable the ADC for interrupts
    ADCIntEnable(ADC0_BASE, 3);
}


/*******************************************************
 * Function: initTimer
 *
 * Initialises a timer interrupt used to poll the ADC
 *******************************************************/
void
initTimer(void)
{
    // Enable TIMER1 peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1));  // busy-wait until TIMER1's bus clock is ready

    // Configure the timer as periodic
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);

    // Set the frequency as SAMPLE_RATE_HZ
    TimerLoadSet(TIMER1_BASE, TIMER_A, SysCtlClockGet() / SAMPLE_RATE_HZ);

    // Register the timerInterrupt handler
    TimerIntRegister(TIMER1_BASE, TIMER_A, timerIntHandler);

    // Enable the periodic timer interrupt
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    // Enable the timer
    TimerEnable(TIMER1_BASE, TIMER_A);
}


/*******************************************************
 * Function: timerIntHandler
 *
 * Periodic timer interrupt used to trigger the ADC
 * interrupt handler.
 * (There is a more efficient way to do this by triggering
 * the ACD interrupt handler directly in TimerIntRegister())
 *******************************************************/
void
timerIntHandler(void)
{
    // Trigger the ADC interrupt handler
    ADCProcessorTrigger(ADC0_BASE, 3);

    //
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
}

/*******************************************************
 * Function: ADCIntHandler
 *
 * Triggered by the timer interrupt and writes the output to a circular buffer
 *******************************************************/
void
ADCIntHandler(void)
{
    // Initialise variable
    uint32_t ulValue;

    // Read data from ACD and store in variable
    ADCSequenceDataGet(ADC0_BASE, 3, &ulValue);

    // Write ADC data to circular buffer
    writeCircBuf(&g_inBuffer, ulValue);

    // Clear the Interrupt
    ADCIntClear(ADC0_BASE, 3);
}


/*******************************************************
 * Function: GetHeightTask
 *
 * Reads the circular buffer and averages the values to get current height
 * Writes the current height to the FreeRTOS Queue OLEDQueue
 *
 * pvParameters: NULL
 *******************************************************/
void
getHeightTask (void *pvParameters)
{
    /* Initialise Task Variables */

    // Cast the queue pointer in pvParameters back into a Queue type
    xQueueHandle Queue = *((xQueueHandle *) pvParameters);

    // Used for averaging the ADC Buffer
    uint16_t i;
    int32_t sum;

    // Used for mapping average ADC value to altitude (linear)
    uint32_t x;
    uint32_t y;
    uint16_t b = 242;
    double m = -0.081;

    // Used for sending a message to the OLED display task
    OLEDMessage Message;


    while(1){

        // Average the ADC values stored in the circular buffer
        sum = 0;
        for (i = 0; i < BUF_SIZE; i++) {
            sum = sum + readCircBuf (&g_inBuffer);
        }
        x = (2 * sum + BUF_SIZE) / 2 / BUF_SIZE; // Averaged Value

        // Adjust average ADC value into altitude reading
        y = (m*x) + b; // Mapped value

        // Store a message into the OLEDMessage string buffer.
        Message.charLine = 1;
        Message.charPos = 0;
        usnprintf(Message.strBuf, sizeof(Message.strBuf), "Height (/): %d ", y);

        // Send the Message to the OLEDQueue
        if (pdTRUE != xQueueSend(Queue, (void*)&Message, 10)) {  // Check if the message can be sent to the LEDQueue, wait 10 ticks
            while(1);  // Can't sent to Queue
        }

        // Delay the task
        vTaskDelay(ADC_DISPLAY_RATE / portTICK_RATE_MS);
    }
}


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
initGetHeightTask(xQueueHandle* OLEDQueue)
{
    initADC();  // Initialise the ADC
    initTimer();  // Initialise interrupt timer
    initCircBuf(&g_inBuffer, BUF_SIZE);  // Initialise the circular buffer

    //Create getHeightTask task
    if (pdTRUE != xTaskCreate(getHeightTask, "Get Height Data", TASK_STACK_DEPTH, (void *) OLEDQueue, TASK_PRIORITY, NULL))
    {
        return(1);  // Fail (Must not have had enough memory to create the task)
    }
    return(0);  // Success
}
