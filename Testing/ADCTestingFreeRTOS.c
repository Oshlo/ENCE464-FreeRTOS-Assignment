/*
 * ADCTestingFreeRTOS.c
 * Implementing ADCTesting.c in freeRTOS
 *
 *  Created on: 3/08/2021
 *      Author: ADAM
 */


#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"

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

#include "utils/uartstdio.h"
#include "utils/ustdlib.h"

#include "circBufT.h"
#include "OrbitOLED/OrbitOLEDInterface.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


#define TASK_STACK_DEPTH    32
#define TASK_PRIORITY       4

#define BUF_SIZE 5
#define ADC_DISPLAY_RATE    200 // (ms)
#define SAMPLE_RATE_HZ      10

static circBuf_t g_inBuffer;        // Buffer of size BUF_SIZE integers (sample values)

#define QUEUE_LENGTH 5
#define ADC_QUEUE_ITEM_SIZE sizeof(int32_t)

static xQueueHandle ADCQueue;


// Prototypes
void initCLK (void);
void initADC (void);
void initTimer (void);
void initDisplay (void);
void initQueue (void);
void timerIntHandler (void);
void ADCIntHandler (void);
void getADCData (void *pvParameters);


void
initCLK(void)
{
    SysCtlClockSet (SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);  // Set the clock rate to 80 MHz
}


void
initADC (void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0));        // busy-wait until ADC0's bus clock is ready

    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);

    ADCSequenceEnable(ADC0_BASE, 3);

    ADCIntRegister (ADC0_BASE, 3, ADCIntHandler);

    ADCIntEnable(ADC0_BASE, 3);
}


void
initTimer(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1));        // busy-wait until TIMER1's bus clock is ready

    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);

    TimerLoadSet(TIMER1_BASE, TIMER_A, SysCtlClockGet() / SAMPLE_RATE_HZ);

    TimerIntRegister(TIMER1_BASE, TIMER_A, timerIntHandler);

    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    TimerEnable(TIMER1_BASE, TIMER_A);
}


void
initDisplay (void)
{
    OLEDInitialise ();

    OLEDStringDraw("ADCTesting.c", 0, 0);
}

void
initQueue (void)
{
    ADCQueue = xQueueCreate(QUEUE_LENGTH, ADC_QUEUE_ITEM_SIZE); // Create the Global LEDQueue
    if (ADCQueue == NULL) {
        while(1); // No Memory for Queue
    }
}


void
timerIntHandler(void)
{
    ADCProcessorTrigger(ADC0_BASE, 3);

    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
}


void
ADCIntHandler(void)
{
    uint32_t ulValue;

    ADCSequenceDataGet(ADC0_BASE, 3, &ulValue);

    writeCircBuf (&g_inBuffer, ulValue);

    ADCIntClear(ADC0_BASE, 3);
}


void getADCData (void *pvParameters)
{
    uint16_t i;
    int32_t sum;
    int32_t filtered;

    while(1){

        sum = 0;
        for (i = 0; i < BUF_SIZE; i++) {
            sum = sum + readCircBuf (&g_inBuffer);
        }

        filtered = (2 * sum + BUF_SIZE) / 2 / BUF_SIZE;

        if (xQueueSend(ADCQueue, (void *)&filtered, 10) != pdPASS)
        {
          while(1); // Could not send to Queue
        }

        vTaskDelay(ADC_DISPLAY_RATE / portTICK_RATE_MS);
    }
}


void
displayADCData(void *pvParameters)
{
    int32_t ADCData; // Create a local count variable to store the LEDQueue Data

    while(1)
    {
        if (xQueueReceive(ADCQueue, (void *)&ADCData, portMAX_DELAY) == pdTRUE) {
            char textBuffer[17]; // Initialize a char string buffer

            //OLEDStringDraw ("                ", 0, 1);
            usnprintf (textBuffer, sizeof(textBuffer), "ADC DATA: %d      ", ADCData);
            OLEDStringDraw (textBuffer, 0, 1);
        }
    }
}


int main(void)
{
    initCLK();
    initADC();
    initTimer();
    initDisplay();
    initCircBuf (&g_inBuffer, BUF_SIZE);
    initQueue();


    if (pdTRUE != xTaskCreate(getADCData, "Get ADC Data", TASK_STACK_DEPTH, NULL, TASK_PRIORITY, NULL))
    {
        while(1);               // Oh no! Must not have had enough memory to create the task.
    }

    if (pdTRUE != xTaskCreate(displayADCData, "Display ADC Data", TASK_STACK_DEPTH, NULL, TASK_PRIORITY, NULL))
        {
            while(1);               // Oh no! Must not have had enough memory to create the task.
        }


    IntMasterEnable();

    vTaskStartScheduler();      // Start FreeRTOS!!

    while(1);                   // Should never get here since the RTOS should never "exit".
}
