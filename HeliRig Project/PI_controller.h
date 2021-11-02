#ifndef _PID_H_
#define _PID_H_

/*******************************************************
 * PI_controller.h
 *
 * Produces PI feedback control based on the present value (pv) and set point (sp)
 *
 *  Created on: 12/08/2021
 *      Author: Group 1
 *******************************************************/


/*******************************************************
 * Constants
 *******************************************************/
#define PIDSTACK 64

struct PID_Struct
{
    //Gain Values
    double kp;
    double ki;

    //Set values
    double dt;
    double max;
    double min;

    //Variables
    double pre_error;
    double integral;

} PID_var;


/*******************************************************
 * Function: pid_calc
 *
 * Creates the feedback term by calculating proportional and integral error
 *
 * set point: the desired position
 * pv: the present value being measured
 *
 * returns: combined proportional and integral feedback value
 *******************************************************/
extern double
calculate(double setpoint, double pv);


/*******************************************************
 * Function: PIDinit
 *
 * Creates the FreeRTOS task PID_CALCULATE
 *
 * dt: time step
 * max: maximum feedback value
 * min: minimum feedback value
 * kp: proportional gain
 * ki: integral gain
 *
 * returns: 0 on successful creation of PID_CALCULATE
 *          1 on failed attempt
 *******************************************************/
extern void
PIDinit(double dt, double max, double min, double kp, double ki);


#endif /* _PID_H_ */
