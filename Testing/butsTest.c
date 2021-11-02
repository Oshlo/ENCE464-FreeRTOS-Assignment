//*****************************************************************************
//
// butsTest.c - Test for the new buttons2 module
//
// Author:  P.J. Bones	UCECE
// Last modified:	11.3.2017
//

#include <stdint.h>
#include <stdbool.h>
#include "utils/ustdlib.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/adc.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/debug.h"
#include "stdio.h"
#include "stdlib.h"
#include "OrbitOLED/OrbitOLEDInterface.h"
#include "buttons4.h"


//*****************************************************************************
// Initialisation functions: clock, display
//*****************************************************************************
void
initClock (void)
{
    // Set the clock rate to 20 MHz
    SysCtlClockSet (SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);
}


// *******************************************************
void
initDisplay (void)
{
  // intialise the Orbit OLED display
	OLEDInitialise ();
}


//*****************************************************************************
//
// Function to display the mean interval in usec
//
//*****************************************************************************
void
displayButtonState (char *butStr, char *stateStr, uint8_t numPushes, uint8_t charLine)
{
    char string[17]; //Display fits 16 characters wide.
	
    OLEDStringDraw ("                ", 0, charLine);
    usnprintf (string, sizeof(string), "%s - %s %d", butStr, stateStr, numPushes);
    OLEDStringDraw (string, 0, charLine);
}


int
main(void)
{
	int8_t upPushes = 0, downPushes = 0;
	
	initClock ();
	initDisplay ();
	initButtons ();
    
    OLEDStringDraw ("buttons test", 0, 0);

    displayButtonState ("UP  ", "RELS", upPushes, 2);
    displayButtonState ("DOWN", "RELS", downPushes, 3);

	while(1)
	{
		uint8_t butState;

        updateButtons ();		// Poll the buttons
        // check state of each button and display if a change is detected
        butState = checkButton (UP);
        switch (butState)
        {
        case PUSHED:
        	displayButtonState ("UP  ", "PUSH", ++upPushes, 2);
        	break;
        case RELEASED:
        	displayButtonState ("UP  ", "RELS", upPushes, 2);
        	break;
        // Do nothing if state is NO_CHANGE
        }
        butState = checkButton (DOWN);
        switch (butState)
        {
        case PUSHED:
        	displayButtonState ("DOWN", "PUSH", ++downPushes, 3);
        	break;
        case RELEASED:
        	displayButtonState ("DOWN", "RELS", downPushes, 3);
        	break;
        // Do nothing if state is NO_CHANGE
        }

		SysCtlDelay (SysCtlClockGet () / 150);	// Approx 50 Hz polling
	}
}

