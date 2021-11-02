/*******************************************************
 * initUART.c
 *
 * A file to initialise the Serial Port, for the use of UARTprintf()
 *
 * Need to ensure that only one task is using UARTprintf() at once.
 * This can be implemented by utilising a mutex. This will have to be done in main.
 *
 *  Created on: 3/08/2021
 *      Author: Group 1
 *******************************************************/


#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"

#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"

#include "utils/uartstdio.h"
#include "utils/ustdlib.h"

#include "initUART.h"


/*******************************************************
 * Function: initUART
 *
 * Configures the UART and its pins
 *      This must be called before UARTprintf()
 *******************************************************/
void
initUART(void)
{
    // Enable the GPIO Peripheral used by the UART.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));  // busy-wait until GPIOA's bus clock is ready

    // Enable UART0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));  // busy-wait until UART0's bus clock is ready

    // Configure GPIO Pins for UART mode.
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Use the internal 16MHz oscillator as the UART clock source.
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    // Configure the UART for console I/O.
    UARTStdioConfig(0, 115200, 16000000);
}
