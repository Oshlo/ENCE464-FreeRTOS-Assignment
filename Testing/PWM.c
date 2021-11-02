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
// #include "buttons.h"
#include "stdio.h"
#include "buttons4.h"

#include "freeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "queue.h"

//**********************************************************
// Generates a single PWM signal on Tiva board pin J4-05 =
// PC5 (M0PWM7).  This is the same PWM output as the
// helicopter main rotor.
//***********************************************************

/*******************************************
 *     PWM config details for main
 *******************************************/
#define PWM_FIXED_RATE_HZ  250 // 250 == 50hz
#define PWM_START_DUTY   0
#define PWM_DIVIDER_CODE  SYSCTL_PWMDIV_4
#define PWM_DIVIDER_CLOCK  4

/*******************************************
 *      PWM Hardware Details M0PWM7 (gen 3)
 *******************************************/
//---Main Rotor PWM: M0PWM7,PC5, J4-05
#define PWM_MAIN_BASE          PWM0_BASE
#define PWM_MAIN_GEN         PWM_GEN_3
#define PWM_MAIN_OUTNUM      PWM_OUT_7
#define PWM_MAIN_OUTBIT      PWM_OUT_7_BIT
#define PWM_MAIN_PERIPH_PWM   SYSCTL_PERIPH_PWM0
#define PWM_MAIN_PERIPH_GPIO SYSCTL_PERIPH_GPIOC
#define PWM_MAIN_GPIO_BASE   GPIO_PORTC_BASE
#define PWM_MAIN_GPIO_CONFIG GPIO_PC5_M0PWM7
#define PWM_MAIN_GPIO_PIN    GPIO_PIN_5

//semaphores and Queues
SemaphoreHandle_t PWMOffSemaphore;
SemaphoreHandle_t PWMOnSemaphore;
SemaphoreHandle_t chaneMainPWMSemaphore;
//tail sem

//Queue for duty cycle
xQueueHandle Q_MainDuty;

//Queue variables
#define DUTY_SIZE          sizeof(uint32_t)
#define DUTY_QUEUE_LENGTH   1

/*******************************************
 *      Local prototypes
 *******************************************/

/*void initClocks (void);
void initSysTick (void);
void initialisePWM (void);
void setPWM (void);*/

/****************************************************
 * initialisePWM
 * M0PWM7 (J4-05, PC5) is used for the main rotor motor
 ***************************************************/
void
initialisePWM (void)
{

    //***PWM timer***
    SysCtlPWMClockSet(PWM_DIVIDER_CODE); //Divides main clock by div code 80M/4 = 20M ticks

    //---Main Rotor--
    SysCtlPeripheralEnable(PWM_MAIN_PERIPH_PWM);
    while (!SysCtlPeripheralReady(PWM_MAIN_PERIPH_PWM));
    SysCtlPeripheralEnable(PWM_MAIN_PERIPH_GPIO);
    while (!SysCtlPeripheralReady(PWM_MAIN_PERIPH_GPIO));


    GPIOPinConfigure(PWM_MAIN_GPIO_CONFIG);
    GPIOPinTypePWM(PWM_MAIN_GPIO_BASE, PWM_MAIN_GPIO_PIN);

    PWMGenConfigure(PWM_MAIN_BASE, PWM_MAIN_GEN,
                    PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);


    //set initial parameters
    PWMGenEnable(PWM_MAIN_BASE, PWM_MAIN_GEN);

    // Disable the output.  Repeat this call with 'true' to turn O/P on.
    PWMOutputState(PWM_MAIN_BASE, PWM_MAIN_OUTBIT, true);

    //create semaphores
    chaneMainPWMSemaphore = xSemaphoreCreateBinary();
    //changeTailPWM;
    PWMOffSemaphore = xSemaphoreCreateBinary();
    PWMOnSemaphore = xSemaphoreCreateBinary();
}

//********************************************************
// Function to set the freq, duty cycle of M0PWM7
//********************************************************
void setPWM (void* pvParameters)
{
    uint32_t ui32Period;
    uint32_t ui32Duty;

    xQueuePeek(Q_MainDuty, &ui32Duty, 0);
                //Calculate the PWM period corresponding to the freq
                ui32Period = configCPU_CLOCK_HZ/PWM_DIVIDER_CLOCK/PWM_FIXED_RATE_HZ; //How many ticks the PWM signal stays high for

    PWMGenPeriodSet(PWM_MAIN_BASE, PWM_MAIN_GEN, ui32Period);
    PWMPulseWidthSet(PWM_MAIN_BASE, PWM_MAIN_OUTNUM, ui32Period * ui32Duty / 100);

    while(1)
    {
        if(pdPASS == xSemaphoreTake(chaneMainPWMSemaphore, 0)) {
            // I literally do not know what is going on here?
        }
    }
}


void
main (void)
{

    //
    // Initialize the buttons
    //
    //ButtonsInit();

    initialisePWM();

    uint32_t Duty = 50;

    Q_MainDuty = xQueueCreate(DUTY_QUEUE_LENGTH, DUTY_SIZE);
    if (Q_MainDuty == NULL) {
        while(1); // Memory
    }

    // Create setPWM task
    if(xTaskCreate(setPWM, "setPWM", 256, NULL, 1, NULL) != pdTRUE) {while(1);}



    if (xQueueSend(Q_MainDuty, (void*)&Duty, 10) != pdPASS)
    {
        while(1); // Cant send to Qeueue
    }


    vTaskStartScheduler();  // Start FreeRTOS


    while(1);
    //Button to change the duty cycle rip from freeRTOS demo in tiva

}


//void
//main(void)
//{
//
//    int first = 1;
//    uint32_t ui32Period;
//    uint32_t ui32Duty = 50;
//
//    initialisePWM();
//
//    ui32Period = configCPU_CLOCK_HZ/PWM_DIVIDER_CLOCK/PWM_FIXED_RATE_HZ/5;
//
//    if (first == 1)
//    {
//        PWMGenPeriodSet(PWM_MAIN_BASE, PWM_MAIN_GEN, ui32Period);
//        PWMPulseWidthSet(PWM_MAIN_BASE, PWM_MAIN_OUTNUM,
//            ui32Period * ui32Duty / 100);
//
//        first++;
//    }
//
//    //PWMOutputState(PWM_MAIN_BASE, PWM_MAIN_OUTBIT, true);
//
//    while(1);
//}

/*int
main (void)
{
    uint32_t ui32Freq = PWM_START_RATE_HZ;

    initClocks ();

    // As a precaution, make sure that the peripherals used are reset
    SysCtlPeripheralReset (PWM_MAIN_PERIPH_GPIO); // Used for PWM output
    SysCtlPeripheralReset (PWM_MAIN_PERIPH_PWM);  // Main Rotor PWM
    SysCtlPeripheralReset (UP_BUT_PERIPH);        // UP button GPIO
    SysCtlPeripheralReset (DOWN_BUT_PERIPH);      // DOWN button GPIO

    initButtons ();
    initialisePWM ();
    initSysTick ();

    // Initialisation is complete, so turn on the output.
    PWMOutputState(PWM_MAIN_BASE, PWM_MAIN_OUTBIT, true);

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
           (ui32Freq < PWM_RATE_MAX_HZ))
       {
              ui32Freq += PWM_RATE_STEP_HZ;
              setPWM ();
       }
       if ((checkButton (DOWN) == PUSHED) && (ui32Freq > PWM_RATE_MIN_HZ))
       {
              ui32Freq -= PWM_RATE_STEP_HZ;
              setPWM ();
       }
    }
}*/



