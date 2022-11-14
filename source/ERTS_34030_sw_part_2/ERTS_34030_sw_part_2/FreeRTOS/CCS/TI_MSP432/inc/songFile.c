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
// File:        songfile.c
// Function:    The music of "Imperial March" from movie "Star Wars".
//              This masterpiece was obtained from the website:
//              http://processors.wiki.ti.com/index.php/Playing_The_Imperial_March
// Credit:      Texas Instruments' wiki

#include "../inc/songFile.h"
#include <stdint.h>
#include "msp.h"
#include "../inc/CortexM.h"
#include "../inc/SysTick.h"

/* Kernel includes. */
#include "FreeRTOS.h"

//Definition of the notes' frequecies in Hertz.
#define c 261
#define d 294
#define e 329
#define f 349
#define g 391
#define gS 415
#define a 440
#define aS 455
#define b 466
#define cH 523
#define cSH 554
#define dH 587
#define dSH 622
#define eH 659
#define fH 698
#define fSH 740
#define gH 784
#define gSH 830
#define aH 880

#define delay_note      500000
#define duration_time   1200
#define delay_beep      600

// Function: init_song_pwm
// Description: initiliase the song
void init_song_pwm()
{
    // configure P2.4 as GPIO output
    P2->SEL0 &= ~0x10;
    P2->SEL1 &= ~0x10;
    P2->DIR |= 0x10;
}

// Function: beep
// Description: This function generates the square wave
//              that makes the piezo speaker sound at a determined frequency.
void beep(unsigned int note, unsigned int duration)
{
    int i;
    // This is the semiperiod of each note.
    long delay = (long)(delay_note/note);

    // This is how much time we need to spend on the note.
    long time = (long)((duration*duration_time)/(delay*2));
    for (i=0;i<time;i++)
    {
        P2->OUT |= 0x10;   // on Set P2.4...
        delay_us(delay);   //...for a semiperiod...
        P2->OUT &= ~0x10;  // off, reset it...
        delay_us(delay);   //...for the other semiperiod.
    }
    delay_ms(delay_beep); //Add a little delay to separate the single notes
}

// Function: play_song
// Description: This is the Imperial March code.
//              As you can see, there are lots of beeps at different frequencies and durations,
//              and some delays to separate the various bits of this wonderful song.
void play_song()
{
    beep(a, 500);
    beep(a, 500);
    beep(a, 500);
    beep(f, 350);
    beep(cH, 150);
    beep(a, 500);
    beep(f, 350);
    beep(cH, 150);
    beep(a, 650);

    delay_ms(150);
    //end of first bit

    beep(eH, 500);
    beep(eH, 500);
    beep(eH, 500);
    beep(fH, 350);
    beep(cH, 150);
    beep(gS, 500);
    beep(f, 350);
    beep(cH, 150);
    beep(a, 650);

    delay_ms(150);
    //end of second bit...

    beep(aH, 500);
    beep(a, 300);
    beep(a, 150);
    beep(aH, 400);
    beep(gSH, 200);
    beep(gH, 200);
    beep(fSH, 125);
    beep(fH, 125);
    beep(fSH, 250);

    delay_ms(250);

    beep(aS, 250);
    beep(dSH, 400);
    beep(dH, 200);
    beep(cSH, 200);
    beep(cH, 125);
    beep(b, 125);
    beep(cH, 250);

    delay_ms(250);

    beep(f, 125);
    beep(gS, 500);
    beep(f, 375);
    beep(a, 125);
    beep(cH, 500);
    beep(a, 375);
    beep(cH, 125);
    beep(eH, 650);

    //end of third bit... (Though it doesn't play well)
    //let's repeat it

    beep(aH, 500);
    beep(a, 300);
    beep(a, 150);
    beep(aH, 400);
    beep(gSH, 200);
    beep(gH, 200);
    beep(fSH, 125);
    beep(fH, 125);
    beep(fSH, 250);

    delay_ms(250);

    beep(aS, 250);
    beep(dSH, 400);
    beep(dH, 200);
    beep(cSH, 200);
    beep(cH, 125);
    beep(b, 125);
    beep(cH, 250);

    delay_ms(250);

    beep(f, 250);
    beep(gS, 500);
    beep(f, 375);
    beep(cH, 125);
    beep(a, 500);
    beep(f, 375);
    beep(cH, 125);
    beep(a, 650);
    //end of the song

    delay_ms(2000); //Add a 2 sec. delay to avoid replaying right after the end.

}

// This two functions stop the main thread for a certain number
//   of milli -or- microseconds.
// They are based on trial and error, but they work fine
//   for the out-of-the-box Launchpad board.
// TI should really add this types of functions as default, just like Arduino does :)

void delay_ms(unsigned int ms )
{
    unsigned int i;
    for (i = 0; i<= ms; i++)
        // Built-in function that suspends the execution for 500 cycles
       __delay_cycles(500);
}

void delay_us(unsigned int us )
{
    unsigned int i;
    for (i = 0; i<= us/2; i++)
       __delay_cycles(1);
}
