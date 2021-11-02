/*******************************************************
 * PI_controller.h
 *
 * Produces PI feedback control based on the present value (pv) and set point (sp)
 *
 *  Created on: 12/08/2021
 *      Author: Group 1
 *******************************************************/

#include <stdio.h>
#include <math.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "PI_controller.h"


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
double
pid_calc (double setpoint, double pv)
{
  double error = setpoint - pv;  // Calculate error

  double Pout = PID_var.kp * error;  // Proportional term

  PID_var.integral += error * PID_var.dt;
  double Iout = PID_var.ki * PID_var.integral;  // Integral term


  double output = Pout + Iout;  // Calculate total output

  /*************************
   * Restrict to max/min
   *    Clamping output to min or max so output does not saturate
   *************************/
  if (output > PID_var.max)
    {
      output = PID_var.max;
    }
  else if (output < PID_var.min)
    {
      output = PID_var.min;
    }

  PID_var.pre_error = error;  // Save error to previous error

  return output;
}


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
uint32_t
PIDinit (double dt, double max, double min, double kp, double ki)
{

    //Initialise values in struct
    PID_var.kp = kp;
    PID_var.ki = ki;
    PID_var.dt = dt;
    PID_var.max = max;
    PID_var.min = min;
    PID_var.pre_error = 0;
    PID_var.integral = 0;

    //Create PID task
    if(xTaskCreate(pid_calc, "PID_CALCULATE", PIDSTACK, NULL, 1, NULL) != pdTRUE)
    {
        return(1);  // Fail (Must not have had enough memory to create the task)
    }

    return(0);  // Success
}


