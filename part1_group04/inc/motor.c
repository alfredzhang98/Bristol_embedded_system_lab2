/*
Simplified BSD License (FreeBSD License)
Copyright (c) 2017, Jonathan Valvano, All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are
those of the authors and should not be interpreted as representing official
policies, either expressed or implied, of the FreeBSD Project.
*/

// Sever VCCMD=VREG jumper on Motor Driver and Power Distribution Board and connect VCCMD to 3.3V.
//   This makes P3.7 and P3.6 low power disables for motor drivers.  0 to sleep/stop.
// Sever nSLPL=nSLPR jumper.
//   This separates P3.7 and P3.6 allowing for independent control
// Left motor direction connected to P1.7 (J2.14)
// Left motor PWM connected to P2.7/TA0CCP4 (J4.40)
// Left motor enable connected to P3.7 (J4.31)
// Right motor direction connected to P1.6 (J2.15)
// Right motor PWM connected to P2.6/TA0CCP3 (J4.39)
// Right motor enable connected to P3.6 (J2.11)

// Author:      Mohd A. Zainol
// Date:        1/10 2018
// Chip:        MSP432P401R LaunchPad Development Kit (MSP-EXP432P401R) for TI-RSLK
// File:        motor.c
// Function:    File for operating the DC motors

#include <stdint.h>
#include "motor.h"
#include "msp.h"
#include "SysTick.h"

#define debug_motor

// *******Lab 12 *******

void Motor_InitSimple(void){
    // initialise P1.6 and P1.7 and make them outputs
    // Returns right away
    P1->SEL0 &= ~0xD2;
    P1->SEL1 &= ~0xD2;        // configure as GPIO
    P1->DIR |= 0x00;          // Motor Forward which is in Module 12.2
    P1->OUT |= 0xD2;          // P1.4 and P1.1 are pull-up
}

void Motor_Stop(){
// Stops both motors, puts driver to sleep
// Returns right away
  P1->OUT &= ~0xC0;   // off
  P2->OUT &= ~0xC0;   // off
}

void Motor_StopSimple(uint32_t time_ms){
// Stops both motors, puts driver to sleep
// Returns right away
  P1->OUT &= ~0xC0;   // off
  P2->OUT &= ~0xC0;   // off
  SysTick_Wait10ms(time_ms); // wait for multiple of 10ms from SysTick
}

/*
	Section: mtr_dir_rgt
	Description: this section is used to turn right of motors
	Port & pin : direction of the left and right motors
	Tips       : for both direction and output, '1' = forward, '0' = backward
	TODO       : finish this section
	(1) Set the direction (DIR) for motor forward of the port
	 	For example: P?->DIR |= 0x??;
	    Where P? is the port number and 0x?? is the pins that will be used
	
	(2) Set the output (OUT) of the port to run the motor
	    For example: P?->OUT |= 0x??;
	    Where P? is the port number and 0x?? is the pins that will be used
*/
void mtr_dir_direction(uint8_t direction){
    switch(direction){
        case STOP:
            P1 -> OUT &= ~0xC0;
            break;
        case BACKWARD:
            P1 -> OUT = (P1 -> OUT & 0x3F) | 0x00;
            break;
        case FORWARD:
            P1 -> OUT = (P1 -> OUT & 0x3F) | 0xC0;
            break;
        case LEFT_FORWARD:
            P1 -> OUT = (P1 -> OUT & 0x3F) | 0x40;
            break;
        case RIGHT_FORWARD:
            P1 -> OUT = (P1 -> OUT & 0x3F) | 0x80;
            break;
        default:
            printf("Wrong direction\n");
    }
    SysTick_Wait1us(10);
}

void mtr_out_control(uint8_t direction){
    switch(direction){
        case STOP:
            P2 -> OUT &= ~0xC0;
            break;
        case BACKWARD:
            P2 -> OUT = (P2 -> OUT & 0x3F) | 0xC0;
            break;
        case FORWARD:
            P2 -> OUT = (P2 -> OUT & 0x3F) | 0xC0;
            break;
        case LEFT_FORWARD:
            P2 -> OUT = (P2 -> OUT & 0x3F) | 0x40;
            break;
        case RIGHT_FORWARD:
            P2 -> OUT = (P2 -> OUT & 0x3F) | 0x80;
            break;
        default:
            printf("Wrong out\n");
    }
    SysTick_Wait1us(10);  
}

/*
	Section mtr_pwm_loop 
	Description: this section is used to turn on/off the motor
	               using PWM of both motors by implementing the duty cycle
	Port & pin : PWM of the left and right motors
    TODO       : finish this section

	use for loop here from 0 to time_ms, count every 1 step
	(1) turn on the PWM of both motors using OUT
	(2) wait for 1us for the duty
	  
        turn of the PWM of both motors
	    P2->OUT &= ~0xC0; 
    (3) wait for 1us for another cycle from the duty
	(4) wait for 1ms using SysTick_Wait
*/
void mtr_pwm_loop(uint16_t high_duty, uint32_t time_ms, uint8_t direction){
      //the loop count
      uint32_t i;
      // The PWM has high (H) and low (L) cycle.
      uint16_t low_duty;

      if(high_duty > 1000){
        printf("Wrong duty input\n");
      }else{
        low_duty =  = 1000 - high_duty;

        //my way of pwm
        // uint32_t pwm_rate = time_ms;
        // uint32_t us_block = (time_ms / pwm_rate) * 1000;
        // float duty_time = high_duty * 1.0 / ((low_duty + high_duty) * 1.0);
        // uint32_t wait_time_high = us_block * duty_time;

        // #ifdef debug_motor
        //     printf("low_duty: %d\n", low_duty);
        //     printf("high_duty:%d\n", high_duty);
        //     printf("duty_time: %f\n", duty_time);
        //     printf("us_block: %d\n", us_block);
        // #endif

        // for (i = 0; i < pwm_rate; i++){
        //     mtr_out_control(direction);
        //     Clock_Delay1us(wait_time_high);
        //     mtr_out_control(STOP);
        //     Clock_Delay1us(us_block - wait_time_high);
        // }
    
        //hint way
        for (i = 0; i < time_ms; i++){
            mtr_out_control(direction);
            Clock_Delay1us(high_duty);
            mtr_out_control(STOP);
            Clock_Delay1us(low_duty);
        }
      }
      SysTick_Wait(1);
}

void Motor_ForwardSimple(uint16_t duty, uint32_t time_ms){
    //init
    Motor_Stop();
    //set dir
    mtr_dir_direction(FORWARD);
    //loop
	mtr_pwm_loop(duty, time_ms, FORWARD);
    //stop
    Motor_Stop();
    //sys delay
    SysTick_Wait(1);
}
void Motor_BackwardSimple(uint16_t duty, uint32_t time_ms){
    Motor_Stop();
    mtr_dir_direction(BACKWARD);
	mtr_pwm_loop(duty, time_ms, BACKWARD);
    Motor_Stop();
    SysTick_Wait(1);
}
void Motor_LeftSimple(uint16_t duty, uint32_t time_ms){
    Motor_Stop();
    mtr_dir_direction(LEFT_FORWARD);
	mtr_pwm_loop(duty, time_ms, LEFT_FORWARD);
    Motor_Stop();
    SysTick_Wait(1);
}
void Motor_RightSimple(uint16_t duty, uint32_t time_ms){
    Motor_Stop();
    mtr_dir_direction(RIGHT_FORWARD);
	mtr_pwm_loop(duty, time_ms, RIGHT_FORWARD);
    Motor_Stop();
    SysTick_Wait(1);
}

/*
    turn: 0x02 LEFT_FORWARD, 0x03 RIGHT_FORWARD
*/
void Motor_Degree(uint8_t turn, uint16_t degree){
    //round duty
    uint16_t round_duty = 500;
    //whole circle round take us
    uint32_t whole_round_us = 1000;
    float percent_round = (degree * 1.0) / (360 * 1.0);
    uint32_t time_round = percent_round * whole_round_us;
    if(turn == LEFT_FORWARD){
        Motor_LeftSimple(round_duty, time_round);
    }else if(turn == RIGHT_FORWARD){
        Motor_RightSimple(round_duty, time_round);
    }else{
        printf("Wrong turn\n");
    }
}