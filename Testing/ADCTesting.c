/*
 * ADCTesting.c
 * Ideally looking to run the ADC off a periodic timer.
 * Its working now, time to make it run in freeRTOS
 *
 *  Created on: 29/07/2021
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

#include "circBuf.h"
#include "OrbitOLED/OrbitOLEDInterface.h"

// prototypes
void initCLK (void);
void initUART (void);
void initADC (void);
void initTimer (void);
void initDisplay (void);
void initPeripherals (void);
void displayMeanVal(uint16_t, uint32_t);
void timerIntHandler (void);
void ADCIntHandler (void);



#define BUF_SIZE 10
#define SAMPLE_RATE_HZ 10

static circBuf_t g_inBuffer;        // Buffer of size BUF_SIZE integers (sample values)
static uint32_t g_ulSampCnt;        // Counter for the interrupts



void
initCLK(void)
{
    SysCtlClockSet (SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);  // Set the clock rate to 80 MHz
}


void
initUART(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    UARTStdioConfig(0, 115200, 16000000);
}


void
initADC (void)
{
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);

    ADCSequenceEnable(ADC0_BASE, 3);

    ADCIntRegister (ADC0_BASE, 3, ADCIntHandler);

    ADCIntEnable(ADC0_BASE, 3);
}


void
initTimer(void)
{
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
}


void
initPeripherals(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
}


void
displayMeanVal(uint16_t meanVal, uint32_t count)
{
    char string[17];  // 16 characters across the display

    OLEDStringDraw ("ADC demo 1", 0, 0);

    // Form a new string for the line.  The maximum width specified for the
    //  number field ensures it is displayed right justified.
    usnprintf (string, sizeof(string), "Mean ADC = %4d", meanVal);
    // Update line on display.
    OLEDStringDraw (string, 0, 1);

    usnprintf (string, sizeof(string), "Sample # %5d", count);
    OLEDStringDraw (string, 0, 3);
}


void
timerIntHandler(void)
{
    ADCProcessorTrigger(ADC0_BASE, 3);
    g_ulSampCnt++;

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


int
main (void)
{
    uint16_t i;
    int32_t sum;

    initCLK();
    initUART();
    UARTprintf("ACD Testing: Hopefully this works\n");
    initPeripherals();
    initADC();
    initTimer();
    initDisplay();
    initCircBuf (&g_inBuffer, BUF_SIZE);


    IntMasterEnable();

    while(1){

        sum = 0;
        for (i = 0; i < BUF_SIZE; i++) {
            sum = sum + readCircBuf (&g_inBuffer);
        }

        displayMeanVal ((2 * sum + BUF_SIZE) / 2 / BUF_SIZE, g_ulSampCnt);

        SysCtlDelay (SysCtlClockGet() / 100);  // Update display at ~ 2 Hz
    }
}
