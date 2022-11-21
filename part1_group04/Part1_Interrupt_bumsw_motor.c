//====================================================================================================================================================================
//==================================================================================Group 04 =========================================================================
//====================================================================================================================================================================

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

// Author:      Mohd A. Zainol, Reza Nejabati
// Date:        10/10/2018
// Chip:        MSP432P401R LaunchPad Development Kit (MSP-EXP432P401R) for TI-RSLK
// File:        Part1_Interrupt_bumpsw_motor.h
// Function:    Part 1 of ERTS, uses interrupt for bump switches to control the motor

#include <stdint.h>
#include "msp.h"
#include "SysTick.h"
#include "motor.h"
#include "Clock.h"
#include "CortexM.h"

// Check these if you really need to include these libraries in your code
/*
#include "../inc/Clock.h"
#include "../inc/SysTick.h"
#include "../inc/CortexM.h"
#include "../inc/motor.h"
*/

// Color    LED(s) Port2
// dark     ---    0
// blue     --B    0x04
// green    -G-    0x02
// sky blue -GB    0x06
// red      R--    0x01
// pink     R-B    0x05
// yellow   RG-    0x03
// white    RGB    0x07
#define RED       0x01
#define GREEN     0x02
#define YELLOW    0x03
#define BLUE      0x04
#define PINK      0x05
#define SKYBLUE   0x06
#define WHITE     0x07

#define MODE_NONE    0xFF
#define MODE_DEFAULT 0x00
#define MODE_SW1     0x01
#define MODE_SW2     0x02

#define MODE_IP_NONE     0x00
#define MODE_INTERRUPT   0x01
#define MODE_POLLING     0x02

//global v
uint8_t mode = MODE_DEFAULT;
uint8_t IP_mode = MODE_IP_NONE;


////PWM timer
// void PWM_init(uint16_t period, uint16_t duty1, uint16_t duty2){
//     P2 -> DIR |= 0x030;
//     P2 -> SEL0 |= 0x030;
//     P2 -> SEL1 &= ~0x030;
//     TIMER_ A0->CCTL[0] = 0x0080;
//     TIMER A0->CCR[0] = period;
//     TIMER A0->EXO = Ox0000;
//     TIMER_AO->CCTL[1] = 0x0040;
//     TIMER_AO->CC[1] = duty1;
//     TIMER_A0->CCTL[2] = 0x0040;
//     TIMER_AO->CCR[2] = duty2;
//     TIMER_A0->CTL = 0x02F0;
// }


// Initialize SW1/SW2 using interrupt
// Make six from Port 4 input pins
// Activate interface pull-up
// The pins are P1.7,P1.6
void SWEdgeTrigger_Init(void){
    P1->SEL0 &= ~0x12;
    P1->SEL1 &= ~0x12;      // configure as GPIO
    P1->DIR &= ~0x12;       // make in
    P1->REN |= 0x12;        // enable pull resistors
    P1->OUT |= 0x12;        // pull-up
    P1->IES |= 0x12;        // falling edge event
    P1->IFG &= ~0x12;       // clear flag
    P1->IE |= 0x12;         // arm the interrupt
    // priority 2 on port1
    NVIC->IP[9] = (NVIC->IP[9]&0xFF00FFFF)|0x00400000;
    // enable interrupt 35 in NVIC on port1
    NVIC->ISER[1] = 0x00000008;
}

// Initialize Bump sensors using interrupt
// Make six from Port 4 input pins
// Activate interface pull-up
// The pins are P4.7, 4.6, 4.5, 4.3, 4.2, 4.0
void BumpEdgeTrigger_Init(void){
    P4->SEL0 &= ~0xED;
    P4->SEL1 &= ~0xED;      // configure as GPIO
    P4->DIR &= ~0xED;       // make in
    P4->REN |= 0xED;        // enable pull resistors
    P4->OUT |= 0xED;        // pull-up
    P4->IES |= 0xED;        // falling edge event
    P4->IFG &= ~0xED;       // clear flag
    P4->IE |= 0xED;         // arm the interrupt
    // priority 2 on port4
    NVIC->IP[9] = (NVIC->IP[9]&0xFF00FFFF)|0x00400000;
    // enable interrupt 38 in NVIC on port4
    NVIC->ISER[1] = 0x00000040;
}

//port1 interrupt
void PORT1_IRQHandler(void){
    uint8_t status;
    status = P1->IV;
    switch(status){
    //SW1 0x04
    case 0x04:
        if(mode != MODE_SW1){
            //reset the motor
            Motor_InitSimple(); 
            Motor_StopSimple(100);
        }
        mode = MODE_SW1;
        break;
    //SW2 0x0A
    case 0x0A:
        if(mode != MODE_SW2){
            //reset the motor
            Motor_InitSimple(); 
            Motor_StopSimple(100);
        }
        mode = MODE_SW2;  
        break;
    }
    P1->IFG &= ~0xED; // clear flag
}

// Uses P4IV IRQ handler to solve critical section/race
void PORT4_IRQHandler(void){

    uint8_t status;

    // The movement for coloured LED
    // WHITE:   Forward
    // BLUE:    Turn right
    // YELLOW:  Turn left
    // GREEN:   Backward
    Port2_Output(0);        // turn off the coloured LED

    // Interrupt Vector of Port4
    status = P4->IV;      // 2*(n+1) where n is highest priority

	  // The case used are the interrupt vector of P4->IV
	  // For example, the bump switch 3 is connected to P4.3
	  // (in other words, Port 4 at pin 3),
	  // thus the status of case would be:
	  // status = 2*(pin number + 1)
	  //        = 2*(pin_3 + 1)
	  //        = 2*(3 + 1)
	  //        = 2*(4)
	  //        = 8
	  // in hex = 0x08
	  // (*NOTE: in this code only bump switch 3 has been calculated,
	  //         please figure out the other bump switches)
//#define MODE_SW2_NONE   0x00
//#define MODE_SW2_LEFT_SHORT   0x01
//#define MODE_SW2_LEFT_LONG    0x02
//#define MODE_SW2_RIGHT_SHORT  0x03
//#define MODE_SW2_RIGHT_LONG   0x04
//#define MODE_SW2_FACE         0x05
//    void Motor_ForwardSimple(uint16_t duty, uint32_t time_ms);
//    void Motor_BackwardSimple(uint16_t duty, uint32_t time_ms);
//    void Motor_LeftSimple(uint16_t duty, uint32_t time_ms);
//    void Motor_RightSimple(uint16_t duty, uint32_t time_ms);
//    void Motor_Degree(uint8_t turn, uint16_t degree);
    if(mode == MODE_SW1){
        mode = MODE_DEFAULT;
        Motor_InitSimple(); 
        Motor_StopSimple(100);
    }
    if(mode == MODE_SW2){
      switch(status){
//      __no_operation();      // the code will run without operation/
      //=========Group 04============
      // Bump switch 1,Bump switch 2,Bump switch 3 on the right of the car
      // Bump switch 4,Bump switch 5,Bump switch 6 on the left of the car

        case 0x02: // Bump switch 1 P4.0
            // Change the coloured LED into green (backward)
            Port2_Output(GREEN);
            // Move backward at 500 duty for 200ms
            Motor_BackwardSimple(500,200);
			// turn off the coloured LED
            Port2_Output(0);
            // Stop for 1000ms
            Clock_Delay1ms(1000);
			// Change the coloured LED into yellow (turn left)
            Port2_Output(YELLOW);
            // Make a left turn at 500 duty for 100ms
            Motor_LeftSimple(500,100);
			// turn off the coloured LED
            Port2_Output(0);
            // Stop for 1000ms
            Clock_Delay1ms(1000);
          break;
        case 0x06: // Bump switch 2 P4.2 6
            // Change the coloured LED into green (backward)
            Port2_Output(GREEN);
            // Move backward at 500 duty for 200ms
            Motor_BackwardSimple(500,200);
			// turn off the coloured LED
            Port2_Output(0);
            // Stop for 1000ms
            Clock_Delay1ms(1000);
			// Change the coloured LED into yellow (turn left)
            Port2_Output(YELLOW);
            // Make a left turn at 500 duty for 200ms
            Motor_LeftSimple(500,200);
			// turn off the coloured LED
            Port2_Output(0);
            // Stop for 1000ms
            Clock_Delay1ms(1000);
          break;
        case 0x08: // Bump switch 3 P4.3 8
            // Change the coloured LED into green (backward)
            Port2_Output(GREEN);
            // Move backward at 500 duty for 200ms
            Motor_BackwardSimple(500,200);
			// turn off the coloured LED
            Port2_Output(0);
            // Stop for 1000ms
            Clock_Delay1ms(1000);
			// Change the coloured LED into yellow (turn left)
            Port2_Output(YELLOW);
            // Make a left turn at 500 duty for 300ms
            Motor_LeftSimple(500,300);
            // turn off the coloured LED
            Port2_Output(0);
            // Stop for 1000ms
            Clock_Delay1ms(1000);
          break;
        case 0x0C: // Bump switch 4 P4.5 12
            // Change the coloured LED into green (backward)
            Port2_Output(GREEN);
            // Move backward at 500 duty for 200ms
            Motor_BackwardSimple(500,200);
            // turn off the coloured LED
            Port2_Output(0);
            // Stop for 1000ms
            Clock_Delay1ms(1000);
			// Change the coloured LED into blue (turn right)
            Port2_Output(BLUE);
            // Make a right turn at 500 duty for 300ms
            Motor_RightSimple(500,300);
            // turn off the coloured LED
            Port2_Output(0);
            // Stop for 1000ms
            Clock_Delay1ms(1000);
          break;
        case 0x0E: // Bump switch 5 P4.6 14
            // Change the coloured LED into green (backward)
            Port2_Output(GREEN);
            // Move backward at 500 duty for 200ms
            Motor_BackwardSimple(500,200);
            // turn off the coloured LED
            Port2_Output(0);
            // Stop for 1000ms
            Clock_Delay1ms(1000);
            // Change the coloured LED into blue (turn right)
            Port2_Output(BLUE);
            // Make a right turn at 500 duty for 200ms
            Motor_RightSimple(500,200);
            // turn off the coloured LED
            Port2_Output(0);
            // Stop for 1000ms
            Clock_Delay1ms(1000);
          break;
        case 0x10: // Bump switch 6 P4.7 16
            // Change the coloured LED into green (backward)
            Port2_Output(GREEN);
            // Move backward at 500 duty for 200ms
            Motor_BackwardSimple(500,200);
            // turn off the coloured LED
            Port2_Output(0);
            // Stop for 1000ms
            Clock_Delay1ms(1000);
            // Change the coloured LED into blue (turn right)
            Port2_Output(BLUE);
            // Make a right turn at 500 duty for 300ms
            Motor_RightSimple(500,100);
            // turn off the coloured LED
            Port2_Output(0);
            // Stop for 1000ms
            Clock_Delay1ms(1000);
          break;

        case 0xED: // none of the switches are pressed
          break;
      }
    }
    P4->IFG &= ~0xED; // clear flag
}

// Read current state of 6 switches
// Returns a 6-bit positive logic result (0 to 63)
// bit 5 Bump5
// bit 4 Bump4
// bit 3 Bump3
// bit 2 Bump2
// bit 1 Bump1
// bit 0 Bump0
uint8_t Bump_Read_Input(void){
  return (P4->IN&0xED); // read P4.7, 4.6, 4.5, 4.3, 4.2, 4.0 inputs
}

// Function: checkbumpswitch(uint8_t status)
// Description: this is an alternative way that you can use, 
//              in which it uses polling method that comes from main function.
//              However it is important to note that:
//              1) the polling method is only useful for small program
//              2) the input mask in switch case (for polling method) is DIFFERENT from the 
//                 Nested Vectored Interrupt Controller (NVIC) which used in interrupt method.
void checkbumpswitch(uint8_t status)
{
    if(mode == MODE_SW1){
        mode = MODE_DEFAULT;
        Motor_InitSimple(); 
        Motor_StopSimple(100);
    }
    if(mode == MODE_SW2){
        switch(status){
        //case 0x02: // Bump switch 1 (for interrupt vector)
            case 0x6D: // Bump 1
                // Change the coloured LED into green (backward)
                Port2_Output(GREEN);
                // Move backward at 500 duty for 200ms
                Motor_BackwardSimple(500,200);
                // turn off the coloured LED
                Port2_Output(0);
                // Stop for 1000ms
                Clock_Delay1ms(1000);
                // Change the coloured LED into yellow (turn left)
                Port2_Output(YELLOW);
                // Make a left turn at 500 duty for 100ms
                Motor_LeftSimple(500,100);
                // turn off the coloured LED
                Port2_Output(0);
                // Stop for 1000ms
                Clock_Delay1ms(1000);
            break;
        //case 0x06: // Bump switch 2 (for interrupt vector)
            case 0xAD: // Bump 2
                // Change the coloured LED into green (backward)
                Port2_Output(GREEN);
                // Move backward at 500 duty for 200ms
                Motor_BackwardSimple(500,200);
                // turn off the coloured LED
                Port2_Output(0);
                // Stop for 1000ms
                Clock_Delay1ms(1000);
                // Change the coloured LED into yellow (turn left)
                Port2_Output(YELLOW);
                // Make a left turn at 500 duty for 200ms
                Motor_LeftSimple(500,200);
                // turn off the coloured LED
                Port2_Output(0);
                // Stop for 1000ms
                Clock_Delay1ms(1000);
            break;
        //case 0x08: // Bump switch 3 (for interrupt vector)
            case 0xCD: // Bump 3
                // Change the coloured LED into green (backward)
                Port2_Output(GREEN);
                // Move backward at 500 duty for 200ms
                Motor_BackwardSimple(500,200);
                // turn off the coloured LED
                Port2_Output(0);
                // Stop for 1000ms
                Clock_Delay1ms(1000);
                // Change the coloured LED into yellow (turn left)
                Port2_Output(YELLOW);
                // Make a left turn at 500 duty for 300ms
                Motor_LeftSimple(500,300);
                // turn off the coloured LED
                Port2_Output(0);
                // Stop for 1000ms
                Clock_Delay1ms(1000);
            break;
        //case 0x0C: // Bump switch 4 (for interrupt vector)
            case 0xE5: // Bump 4
                // Change the coloured LED into green (backward)
                Port2_Output(GREEN);
                // Move backward at 500 duty for 200ms
                Motor_BackwardSimple(500,200);
                // turn off the coloured LED
                Port2_Output(0);
                // Stop for 1000ms
                Clock_Delay1ms(1000);
                // Change the coloured LED into blue (turn right)
                Port2_Output(BLUE);
                // Make a right turn at 500 duty for 300ms
                Motor_RightSimple(500,300);
                // turn off the coloured LED
                Port2_Output(0);
                // Stop for 1000ms
                Clock_Delay1ms(1000);
            break;
        //case 0x0E: // Bump switch 5 (for interrupt vector)
            case 0xE9: // Bump 5
                // Change the coloured LED into green (backward)
                Port2_Output(GREEN);
                // Move backward at 500 duty for 200ms
                Motor_BackwardSimple(500,200);
                // turn off the coloured LED
                Port2_Output(0);
                // Stop for 1000ms
                Clock_Delay1ms(1000);
                // Change the coloured LED into blue (turn right)
                Port2_Output(BLUE);
                // Make a right turn at 500 duty for 200ms
                Motor_RightSimple(500,200);
                // turn off the coloured LED
                Port2_Output(0);
                // Stop for 1000ms
                Clock_Delay1ms(1000);
            break;
        //case 0x10: // Bump switch 6 (for interrupt vector)
            case 0xEC: // Bump 6
                // Change the coloured LED into green (backward)
                Port2_Output(GREEN);
                // Move backward at 500 duty for 200ms
                Motor_BackwardSimple(500,200);
                // turn off the coloured LED
                Port2_Output(0);
                // Stop for 1000ms
                Clock_Delay1ms(1000);
                // Change the coloured LED into blue (turn right)
                Port2_Output(BLUE);
                // Make a right turn at 500 duty for 300ms
                Motor_RightSimple(500,100);
                // turn off the coloured LED
                Port2_Output(0);
                // Stop for 1000ms
                Clock_Delay1ms(1000);
            break;
        case 0xED: // neither switch pressed
            break;
        }
    }
}

void Port1_Init(void){
  P1->SEL0 &= ~0x01;
  P1->SEL1 &= ~0x01;        // configure P1.0 as GPIO
  P1->DIR |= 0x01;          //make P1.0 out, the built-in LED1
}

void Port2_Init(void){
    P2->SEL0 &= ~0xC7;
    P2->SEL1 &= ~0xC7;        // configure P2.2 P2.1 P2.0, and P2.6 P2.7 as GPIO
    P2->DIR |= 0x07;          // make P2.2-P2.0 out
    P2->DS |= 0x07;           // activate increased drive strength
    P2->OUT &= ~0x07;         // all LEDs off
    P2->DIR |= 0xC0;          // Direction of the motor
}

void Port2_Output(uint8_t data){
    // built-in red LED connected to P2.0
    // built-in green LED connected to P2.1
    // built-in blue LED connected to P2.2
    // write three outputs bits of P2
    P2->OUT = (P2->OUT&0xF8)|data;
}

void Switch_Init(void){
    // negative logic built-in Button 1 connected to P1.1
    // negative logic built-in Button 2 connected to P1.4
    P1->SEL0 &= ~0x12;
    P1->SEL1 &= ~0x12;      // configure P1.4 and P1.1 as GPIO
    P1->DIR &= ~0x12;       // make P1.4 and P1.1 in
    P1->REN |= 0x12;        // enable pull resistors on P1.4 and P1.1
    P1->OUT |= 0x12;        // P1.4 and P1.1 are pull-up
}

// bit-banded addresses, positive logic
#define SW1IN ((*((volatile uint8_t *)(0x42098004)))^1)
#define SW2IN ((*((volatile uint8_t *)(0x42098010)))^1) // input: switch SW2
#define REDLED (*((volatile uint8_t *)(0x42098040)))    // output: red LED

int main(void){
  uint8_t status;

  Clock_Init48MHz();        // Initialise clock with 48MHz frequency
  SysTick_Init();           // Initialise SysTick timer
  Port1_Init();             // Initialise P1.1 and P1.4 built-in buttons
  Port2_Init();             // Initialise P2.2-P2.0 built-in LEDs
  Motor_InitSimple();       // Initialise DC Motor
  Motor_StopSimple(100);    // Stop the motor on initial state


  REDLED = 1;               // Turn off the red LED
  while(!SW2IN && !SW1IN);
  REDLED = 0;               // Turn off the red LED
  printf("SW1IN: %d\n", SW1IN);
  printf("SW2IN: %d\n", SW2IN);
  if(SW1IN){
      IP_mode = MODE_INTERRUPT;
      BumpEdgeTrigger_Init();   // Initialise bump switches using edge interrupt
  }
  if(SW2IN){
      IP_mode = MODE_POLLING;
    //   Switch_Init();
  }
  SWEdgeTrigger_Init();     // Initialise Switches to interrupt
  EnableInterrupts();       

  // Run forever
  while(1){
      //This is the interrupt function
      if(IP_mode == MODE_INTERRUPT){
          //manual loop interrupts
          __no_operation();
          switch(mode){
          case MODE_DEFAULT:
              SysTick_Wait(100);
              break;
          case MODE_SW1:
              //gourp04 route
              Motor_Route();
              break;
          case MODE_SW2:
              //keep forward
              //Motor_KeepForwardFullSpeed();
              Motor_ForwardSimple(500,1);
              break;
          }
      }
      //This is the polling function
      if(IP_mode == MODE_POLLING){
          //manual loop interrupts
          __no_operation();
          switch(mode){
          case MODE_DEFAULT:
              Motor_StopSimple(100);
              break;
          case MODE_SW1:
              Motor_Route();
              status = Bump_Read_Input();
              if (status == 0x02|| status == 0x06 || status == 0x08 || status == 0x0C || status == 0x0E || status == 0x10) {
                checkbumpswitch(status);
              }
              break;
          case MODE_SW2:
              Motor_ForwardSimple(500,1);
              status = Bump_Read_Input();
              if (status == 0x6D || status == 0xAD || status == 0xCD || status == 0xE5 || status == 0xE9 || status == 0xEC) {
                  checkbumpswitch(status);
              }
              break;
          }
      }
  }
}

//Questions
/*
1. the interrupt takes many time, so the CPU is so high, can we use a timer to realise the PWM or use more flags in the interrupts to realise the same function.
*/