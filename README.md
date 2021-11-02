# **HeliRig Project Group 1**
## Members:
> Alex Scott<br>
> Dael Summerhays<br>
> Harrison Pollard<br>
> Adam Finlayson<br>
***

Project Scope and Instructions can be found [here](ENCE464_T3_project_v2_2021.pdf).</br>
Built with Code Composer Studio (CCS).</br>
**Note:** Project unfinished, capped by COVID19. No access to labs.


## Requirements:
*Our requirements must be a list of **WHAT** the HeliRig does. All requirements must be **Unambiguous** and **Testable**. Think statementents like **"The HeliRIg shall ..."**, and **"If [x], then [y]"**.*<br>

### Hard
1. If the UP button is pressed, the HeliRig's expected height shall increase by 5% (35mm), unless the current HeliRig height is at 100% (70mm).
2. If the DOWN button is pressed, the HeliRig's expected height shall decrease by 5% (35mm), unless the current HeliRig height is at 0% (0mm).
3. If the LEFT button is pressed, the HeiRig's expected yaw shall increase by 10&deg;.
4. If the RIGHT button is pressed, the HelRig's expected yaw shall decrease by 10&deg;.
5. The HeliRig shall move to the expected height and yaw within 5 seconds.
6. The HeliRig shall display it's current height, current yaw, expected height, and expected yaw on the OLED display. It shall update the OLED display at (a minimum) 20Hz.

### Soft
1. If the HeliRig has a current height of 0% (0mm), it shall enter the "Landed" mode.
2. If SWITCH 1 is ON, the HeliRig shall enter the "Hover" mode and hold its position, regardless of expected height and yaw.
3. If SWITCH 1 is OFF, the HeliRig shall enter the "Follow" mode and move to the expected height and yaw within 5 seconds.
4. The HeliRig shall display the curent mode on the OLED display.
5. If the HeliRig encounters an error, it should display details about the error to the serial port. **(This one is pretty ambiguous and hard to test, needs refining)**.

***Incomplete!***
*We need to add more requirements so we have a clear definition of project success.*
***

## TODO List:
*Keep track of all of the important things we have done and need to do. This will make writing the report way easier.*

### One Off
- [ ] Flesh out the requirements enough to define project success.
- [ ] Test PWM output in the HeliRig emulator, and record data to use in the PI controllers.
- [ ] Make a (few) state chart(s) that show the design of our HeliRig software.
    - Software design is the middle layer of ***HOW*** the HeliRig works.
    - Design documents class, task, fuction/method, parameter, and variable names.
- [x] Get the display task working.
    - Ensure it is protected from inteupts. Turn off the master interput routine when calling OLEDStringDraw().
    - Ensure the display task is only reading from one Queue. Use a structure to send all the required data in one go.
- [x] Convert the yaw read to run on interupts.
    - This is important to ensure we do not become out of sync with the emulator.
- [x] Put the Quadrature Decoding into its own module.
- [ ] Poll the buttons using a the timer interupt.
    - This will help with debouncing.
- [ ] Set the correct BITs on the timer interupt to process the ACD data without having to call an external interupt handler.
- [x] Put the ADC into its own module
- [x] Set up the UART Serial Port stuff.
    - Make sure it is protected with the use of semophores/mutexes.
- [x] Format the project properly. 
- [x] Get the quadrature decoding working in freeRTOS
- [x] Get the ADC working with interupts in freeRTOS
- [x] Get test code working for Queues and Semophores/mutexs
- [x] Get test code working for OLED display
- [x] Get test code woring for the PWM

### On Going
- Deside on and start documenting the software archetecture.
    - Software architecture is the outer most layer of ***HOW*** the HeliRig works.
    - It is best documented with a collection of block diagrams. Identify data flow and hardware vs firmware partitions.
    - No confusing details.
- Decide on and use a consistent coding syntax/style.
- Real Time Scheduling analysis.
***

## Tasks:


### get_height_task
- Input: ADC data (in volts)
- Output: Height (in m) to the OLEDQueue, HeightControllerQueue

### PI_controller
- Input: Height (in m) from HeightControllerQueue AND change in Commanded Height from HeightButtonQueue
- PI/PID algorithm
- Output: Commanded motor voltage (in volts) to motor

### yawRead
- Input: Raw yaw data (in ?) from ?
- Calculation
- Output: Yaw (in deg) to the displayQueue, YawControllerQueue

### yawController
- Input: Yaw (in deg) from YawControllerQueue AND change in Commanded Yaw from YawButtonQueue
- PI/PID algorithm
- Output: Commanded motor voltage (in volts) to motor

### OLEDDisplayTask
- Input: Reads from a Queue containing a string buffer 17 chars long (strBuf), and a uint8_t character line (charLine).
- Output: Display the required string onto the specified line on the Orbit OLED Display.

### buttons4
- Input: Increment/Decrement Height/Yaw as interrupts
- Output: Number of +- (as a percentage? or as an integer?) to HeightButtonQueue/YawButtonQueue
***

## For the .cproject merge issue:
A merge conflict is created because we change the files that are being excluded from the ccs debugger. The information about the excluded files is held in the .cproject file.<br>
To stop the merge conflict, make sure that the only files being excluded are in the "Testing" folder. There shouldn't be any reason to exclude files in the "HeliRig Project" folder.<br>
Alternativly, don't worry about alingning the excluded files.
just use <git merge --strategy-option ours> if a merge conflict occurs.
***

## Style Guidelines
File Names = lowercase_with_underscore
Function Names = camelCase

Two spaces between end of line and an in-line comment

'#DEFINE' in the .h file, '#INCLUDE' in the .c file (as per Phil Bones)

'#ifndef' comes before the header comment in .h files (as per Phil Bones)
***

## Things Dael found while formatting and wasn't sure how to handle:
- getHeightTask takes in *pvParameters but doesn't seem to use at any point? (same with OLED_display_task)
- Can the commented out code in PI_controller be deleted?
- "xQueueHandle OLEDQueue; // Global Queue (Not ideal)" from OLED_display_task should be (i think?) moved to the .h file
- Why is queueStructs.c included in OLED-disaply_task (vs the .h file - why isn't there a .h file?)
- main is currently uint32 but doesn't have any return statements 

***

## User Created Files
- All files in HeliRigProject
- queueTesting.c
- ADCTesting.c
- ADCTestingFreeRTOS.c