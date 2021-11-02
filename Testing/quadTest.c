//*****************************************************************************
//
// ADCdemo1.c - Simple interrupt driven program which samples with AIN0
//
// Author:  P.J. Bones  UCECE
// Last modified:   8.2.2018
//
//*****************************************************************************
// Based on the 'convert' series from 2016
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/pwm.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"
#include "utils/ustdlib.h"
#include "circBuf.h"
#include "OrbitOLED/OrbitOLEDInterface.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "get_height_task.h"
#include "quadTest.h"
#include "queue_structs.c"

extern xQueueHandle OLEDQueue;

// Structure used by the Quad
typedef struct Quad_Type
{
    uint8_t A;
    uint8_t B;
    uint8_t L;
    uint8_t R;
    uint8_t Ap;
    uint8_t Bp;
    int32_t sum;

} QuadType;

static volatile QuadType QuadData;

void
quadIntHandler(void)
{
    QuadData.A = (GPIOPinRead (GPIO_PORTB_BASE, GPIO_PIN_0) == GPIO_PIN_0);
    QuadData.B = (GPIOPinRead (GPIO_PORTB_BASE, GPIO_PIN_1) == GPIO_PIN_1);
    QuadData.L = (QuadData.A^QuadData.Bp);
    QuadData.R = (QuadData.B^QuadData.Ap);
    QuadData.sum = QuadData.sum - QuadData.L + QuadData.R;
    QuadData.Ap = QuadData.A;
    QuadData.Bp = QuadData.B;

    GPIOIntClear(GPIO_PORTB_BASE, GPIO_INT_PIN_0);
    GPIOIntClear(GPIO_PORTB_BASE, GPIO_INT_PIN_1);
}

void
initGPIOInt (void)
{
    //
    // The ADC0 peripheral must be enabled for configuration and use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));  // busy-wait until GPIOB's bus clock is ready

    GPIOPinTypeGPIOInput (GPIO_PORTB_BASE, GPIO_PIN_0);
    GPIOPinTypeGPIOInput (GPIO_PORTB_BASE, GPIO_PIN_1);

    GPIOIntRegister(GPIO_PORTB_BASE, quadIntHandler);

    GPIOIntTypeSet(GPIO_PORTB_BASE, GPIO_INT_PIN_0, GPIO_BOTH_EDGES);
    GPIOIntTypeSet(GPIO_PORTB_BASE, GPIO_INT_PIN_1, GPIO_BOTH_EDGES);

    GPIOIntEnable(GPIO_PORTB_BASE, GPIO_INT_PIN_0);
    GPIOIntEnable(GPIO_PORTB_BASE, GPIO_INT_PIN_1);

}



void getQuadData (void *pvParameters)
{
    int32_t angle;

    OLEDMessage OLEDMessage;
    OLEDMessage.charLine = 2;
    OLEDMessage.charPos = 0;

    while (1)
    {
        angle = (int32_t)((long) QuadData.sum * 0.8);
        if (angle > 359){
            angle = 0;
            QuadData.sum = -1;
        }
        if (angle < 0){
            angle = 359;
            QuadData.sum = 449;
        }

        usnprintf(OLEDMessage.strBuf, sizeof(OLEDMessage.strBuf), "Angle: %d     ", angle);

        // Calculate and display the rounded mean of the buffer contents
        if (xQueueSend(OLEDQueue, (void *)&OLEDMessage, 10) != pdPASS)
        {
            while(1); // Could not send to Queue
        }

            vTaskDelay(100 / portTICK_RATE_MS);
    }
}




uint8_t
initQuadTest(void)
{
    initGPIOInt();
    QuadData.Ap = 0;
    QuadData.Bp = 0;
    QuadData.sum = 1;

    if (pdTRUE != xTaskCreate(getQuadData, "Get Quad Data", TASK_STACK_DEPTH, NULL, TASK_PRIORITY, NULL))
    {
        return(1);               // Oh no! Must not have had enough memory to create the task.
    }

    return(0);

}
