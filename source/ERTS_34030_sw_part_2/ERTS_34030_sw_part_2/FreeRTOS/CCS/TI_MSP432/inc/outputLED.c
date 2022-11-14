/*
  This code accompanies the books
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
       ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2017
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
       ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2017
   "Embedded Systems: Real-Time Operating Systems for ARM Cortex-M Microcontrollers",
       ISBN: 978-1466468863, , Jonathan Valvano, copyright (c) 2017
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/

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

// Author:      Mohd A. Zainol
// Date:        1 Oct 2018
// Chip:        MSP432P401R LaunchPad Development Kit (MSP-EXP432P401R) for TI-RSLK
// File:        outputLED.c
// Function:    The definition of two LEDs on the board (red LED and coloured LED).

#include <stdint.h>
#include "msp.h"
#include "Clock.h"
#include "SysTick.h"
#include "CortexM.h"

#include "outputLED.h"
#include "bumpSwitch.h"

/* Kernel includes. */
#include "FreeRTOS.h"

// Function: RedLED_Init
// Description: initialise the built-in LED1 connected to P1.0
void RedLED_Init(void){
  P1->SEL0 &= ~0x01;
  P1->SEL1 &= ~0x01;   // 1) configure P1.0 as GPIO
  P1->DIR |= 0x01;     // 2) make P1.0 out
}

// Function: ColorLED_Init
// Description: initialise the colour LED
void ColorLED_Init(void){
  P2->SEL0 &= ~0x07;
  P2->SEL1 &= ~0x07;    // 1) configure P2.2-P2.0 as GPIO
  P2->DIR |= 0x07;      // 2) make P2.2-P2.0 out
  P2->DS |= 0x07;       // 3) activate increased drive strength
  P2->OUT &= ~0x07;     //    all LEDs off
}

// Function: Port2_Output2
// Description: display the colour LED based on value to represent colour
void Port2_Output2(unsigned char data){  // write three outputs bits of P2
  P2->OUT = (P2->OUT&0xF8)|data;
}

// Function: outputLED_response
// Description: the response of coloured LED when the bump switches are hit
void outputLED_response(unsigned char bumpSwitch_status){

    static const TickType_t xShortBlock = pdMS_TO_TICKS( 300 );
    int i;

    switch(bumpSwitch_status){
      case 0xEC: // Bump switch 1
          Port2_Output2(SKYBLUE);
        break;
      case 0xE9: // Bump switch 2
          Port2_Output2(RED);
        break;
      case 0xE5: // Bump switch 3
          Port2_Output2(PINK);
        break;
      case 0xCD: // Bump switch 4
          Port2_Output2(YELLOW);
        break;
      case 0xAD: // Bump switch 5
          Port2_Output2(GREEN);
        break;
      case 0x6D: // Bump switch 6
          Port2_Output2(BLUE);
        break;
      case 0xED: // neither switch pressed
          Port2_Output2(COLOUROFF);
        break;
      default:
          Port2_Output2(WHITE);
    }
    for (i=0; i<100000; i++);
    Port2_Output2(COLOUROFF);
}

