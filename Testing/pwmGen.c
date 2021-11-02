//**********************************************************
//
// pwmGen.c - Example code which generates a single PWM output
//            on J4-05 (M0PWM7) with duty cycle fixed and the
//            frequency controlled by UP and DOWN buttons in
//            the range 50 Hz to 400 Hz.
// 2017:  Modified for Tiva and using straightforward, polled
//        button debouncing implemented in 'buttons2' module.
//
// P.J. Bones   UCECE
// Last modified:  20.3.2017
//**********************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/pin_map.h" //Needed for pin configure
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "stdio.h"
#include "buttons4.h"

//**********************************************************
// Generates a single PWM signal on Tiva board pin J4-05 =
// PC5 (M0PWM7).  This is the same PWM output as the  
// helicopter main rotor.
//***********************************************************

//***********************************************************
// Constants
//***********************************************************
#define SYSTICK_RATE_HZ  100

/*******************************************
 *      PWM config details.
 *******************************************/
#define PWM_HZ  250
#define PWM_DS_START       50
#define PWM_DS_STEP        1
#define PWM_DS_MIN         20
#define PWM_DS_MAX         80
#define PWM_DIVIDER_CODE  SYSCTL_PWMDIV_4
#define PWM_DIVIDER  4

/*******************************************
 *      PWM Hardware Details M0PWM7 (gen 3)
 *******************************************/
//---Main Rotor PWM: M0PWM7,PC5, J4-05
#define HEIGHT_MAIN_BASE	 PWM0_BASE
#define HEIGHT_MAIN_GEN         PWM_GEN_3
#define HEIGHT_MAIN_OUTNUM      PWM_OUT_7
#define HEIGHT_MAIN_OUTBIT      PWM_OUT_7_BIT
#define HEIGHT_MAIN_PERIPH_PWM	  SYSCTL_PERIPH_PWM0
#define HEIGHT_MAIN_PERIPH_GPIO SYSCTL_PERIPH_GPIOC
#define HEIGHT_GPIO_BASE   GPIO_PORTC_BASE
#define HEIGHT_GPIO_CONFIG GPIO_PC5_M0PWM7
#define HEIGHT_GPIO_PIN    GPIO_PIN_5

#define YAW_MAIN_BASE     PWM1_BASE
#define YAW_MAIN_GEN         PWM_GEN_2
#define YAW_MAIN_OUTNUM      PWM_OUT_5
#define YAW_MAIN_OUTBIT      PWM_OUT_5_BIT
#define YAW_MAIN_PERIPH_PWM    SYSCTL_PERIPH_PWM1
#define YAW_MAIN_PERIPH_GPIO SYSCTL_PERIPH_GPIOF
#define YAW_GPIO_BASE   GPIO_PORTF_BASE
#define YAW_GPIO_CONFIG GPIO_PF1_M1PWM5
#define YAW_GPIO_PIN    GPIO_PIN_1

/*******************************************
 *      Local prototypes
 *******************************************/
void SysTickIntHandler (void);
void initClocks (void);
void initSysTick (void);
void initialisePWM (void);
void setPWM (uint32_t u32Freq, uint32_t u32Duty, uint32_t type);


//***********************************************************
// ISR for the SysTick interrupt (used for button debouncing).
//***********************************************************
void
SysTickIntHandler (void)
{
	//
	// Poll the buttons
	updateButtons();
	//
	// It is not necessary to clear the SysTick interrupt.
}

//***********************************************************
// Initialisation functions: clock, SysTick, PWM
//***********************************************************
// Clock
//***********************************************************
void
initClocks (void)
{
    // Set the clock rate to 20 MHz
    SysCtlClockSet (SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);

    // Set the PWM clock rate (using the prescaler)
    SysCtlPWMClockSet(PWM_DIVIDER_CODE);
}

//*************************************************************
// Initialise the SysTick interrupt
//***********************************************************
void
initSysTick (void)
{
    //
    // Set up the period for the SysTick timer.  The SysTick timer period is
    // set as a function of the system clock.
    SysTickPeriodSet (SysCtlClockGet() / SYSTICK_RATE_HZ);
    //
    // Register the interrupt handler
    SysTickIntRegister (SysTickIntHandler);
    //
    // Enable interrupt and device
    SysTickIntEnable ();
    SysTickEnable ();
}

/****************************************************
 * initialisePWM
 * M0PWM7 (J4-05, PC5) is used for the main rotor motor
 ***************************************************/
void
initialisePWM (void)
{
    SysCtlPeripheralEnable(HEIGHT_MAIN_PERIPH_PWM);
    SysCtlPeripheralEnable(HEIGHT_MAIN_PERIPH_GPIO);

    SysCtlPeripheralEnable(YAW_MAIN_PERIPH_PWM);
    SysCtlPeripheralEnable(YAW_MAIN_PERIPH_GPIO);

    GPIOPinConfigure(HEIGHT_GPIO_CONFIG);
    GPIOPinTypePWM(HEIGHT_GPIO_BASE, HEIGHT_GPIO_PIN);

    GPIOPinConfigure(YAW_GPIO_CONFIG);
    GPIOPinTypePWM(YAW_GPIO_BASE, YAW_GPIO_PIN);

    PWMGenConfigure(HEIGHT_MAIN_BASE, HEIGHT_MAIN_GEN,
                    PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

    PWMGenConfigure(YAW_MAIN_BASE, YAW_MAIN_GEN,
                        PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
    // Set the initial PWM parameters
    setPWM (PWM_HZ, PWM_DS_START, 1);
    setPWM (PWM_HZ, PWM_DS_START, 2);

    PWMGenEnable(HEIGHT_MAIN_BASE, HEIGHT_MAIN_GEN);

    PWMGenEnable(YAW_MAIN_BASE, YAW_MAIN_GEN);

    // Disable the output.  Repeat this call with 'true' to turn O/P on.
    PWMOutputState(HEIGHT_MAIN_BASE, HEIGHT_MAIN_OUTBIT, false);
    PWMOutputState(YAW_MAIN_BASE, YAW_MAIN_OUTBIT, false);
}

//********************************************************
// Function to set the freq, duty cycle of M0PWM7
//********************************************************
void
setPWM (uint32_t ui32Freq, uint32_t ui32Duty, uint32_t type)
{
    // Calculate the PWM period corresponding to the freq.
    uint32_t ui32Period = SysCtlClockGet() / PWM_DIVIDER /                          ui32Freq;

    if (type==1)
    {
        PWMGenPeriodSet(HEIGHT_MAIN_BASE, HEIGHT_MAIN_GEN, ui32Period);
        PWMPulseWidthSet(HEIGHT_MAIN_BASE, HEIGHT_MAIN_OUTNUM,
                ui32Period * ui32Duty / 100);
    }
    if (type==2)
    {
        PWMGenPeriodSet(YAW_MAIN_BASE, YAW_MAIN_GEN, ui32Period);
        PWMPulseWidthSet(YAW_MAIN_BASE, YAW_MAIN_OUTNUM,
                   ui32Period * ui32Duty / 100);
    }

}



int
main (void)
{
    uint32_t ui32DutyH = PWM_DS_START;
    uint32_t ui32DutyY = PWM_DS_START;

    initClocks ();

    // As a precaution, make sure that the peripherals used are reset
    SysCtlPeripheralReset (HEIGHT_MAIN_PERIPH_GPIO); // Used for PWM output
    SysCtlPeripheralReset (HEIGHT_MAIN_PERIPH_PWM);  // Main Rotor PWM
    SysCtlPeripheralReset (YAW_MAIN_PERIPH_GPIO); // Used for PWM output
    SysCtlPeripheralReset (YAW_MAIN_PERIPH_PWM);

    SysCtlPeripheralReset (UP_BUT_PERIPH);        // UP button GPIO
    SysCtlPeripheralReset (DOWN_BUT_PERIPH);      // DOWN button GPIO
    SysCtlPeripheralReset (LEFT_BUT_PERIPH);        // UP button GPIO
    SysCtlPeripheralReset (RIGHT_BUT_PERIPH);      // DOWN button GPIO

    initButtons ();
    initialisePWM ();
    initSysTick ();

    // Initialisation is complete, so turn on the output.
    PWMOutputState(HEIGHT_MAIN_BASE, HEIGHT_MAIN_OUTBIT, true);
    PWMOutputState(YAW_MAIN_BASE, YAW_MAIN_OUTBIT, true);

    //
    // Enable interrupts to the processor.
    IntMasterEnable ();

    //
    // Loop forever, controlling the PWM frequency and
    // maintaining the the PWM duty cycle.
    while (1)
    {
       // Background task: Check for button pushes and control
       // the PWM frequency within a fixed range.
       if ((checkButton (UP) == PUSHED) && 
           (ui32DutyH < PWM_DS_MAX))
       {
    	      ui32DutyH += PWM_DS_STEP;
    	      setPWM (PWM_HZ, ui32DutyH, 1);
       }
       if ((checkButton (DOWN) == PUSHED) && (ui32DutyH > PWM_DS_MIN))
       {
              ui32DutyH -= PWM_DS_STEP;
    	      setPWM (PWM_HZ, ui32DutyH, 1);
       }
       if ((checkButton (LEFT) == PUSHED) && (ui32DutyY > PWM_DS_MIN))
              {
                     ui32DutyY -= PWM_DS_STEP;
                     setPWM (PWM_HZ, ui32DutyY, 2);
              }
       if ((checkButton (RIGHT) == PUSHED) && (ui32DutyY > PWM_DS_MIN))
              {
                     ui32DutyY += PWM_DS_STEP;
                     setPWM (PWM_HZ, ui32DutyY, 2);
              }
    }
}
