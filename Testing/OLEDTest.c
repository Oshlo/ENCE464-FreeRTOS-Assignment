//*****************************************************************************
//
// OLEDTest.c - Test for the Orbit OLED display
//
// Note: uses the string conversion routines in ustdlib.c
//
// Author:  P.J. Bones	UCECE
// Last modified:	8.2.2018
//

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/adc.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/debug.h"
#include "utils/ustdlib.h"
#include "stdlib.h"
#include "OrbitOLED/OrbitOLEDInterface.h"


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
    // Initialise the Orbit OLED display
	OLEDInitialise ();
}


//*****************************************************************************
// Function to display a changing message on the display.
// The display has 4 rows of 16 characters, with 0, 0 at top left.
//*****************************************************************************
void
displayUpdate (char *str1, char *str2, uint8_t num, uint8_t charLine)
{
    char text_buffer[16];           //Display fits 16 characters wide.
	
    // "Undraw" the previous contents of the line to be updated.
    OLEDStringDraw ("                ", 0, charLine);
    // Form a new string for the line.  The maximum width specified for the
    //  number field ensures it is displayed right justified.
    usnprintf(text_buffer, sizeof(text_buffer), "%s %s %3d", str1, str2, num);
    // Update line on display.
    OLEDStringDraw (text_buffer, 0, charLine);
}


int
main(void)
{
	int8_t count = 0;
	
	initClock ();
	initDisplay ();
    
    OLEDStringDraw ("OLED test", 0, 0);
    OLEDStringDraw ("Line 2 . . . . .", 0, 1);
    OLEDStringDraw ("Line 3 . . . . .", 0, 2);
    OLEDStringDraw ("Line 4 . . . . .", 0, 3);

	while(1)
	{
        SysCtlDelay (SysCtlClockGet () / 6);    // Approx 2 Hz
        displayUpdate ("OLED", "test", count++, 2);
	}
}

