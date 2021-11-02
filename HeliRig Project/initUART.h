#ifndef __INIT_UART_H__
#define __INIT_UART_H__

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


/*******************************************************
 * Function: initUART
 *
 * Configures the UART and its pins
 *      This must be called before UARTprintf()
 *******************************************************/
void initUART(void);


#endif /* __INIT_UART_H__ */
