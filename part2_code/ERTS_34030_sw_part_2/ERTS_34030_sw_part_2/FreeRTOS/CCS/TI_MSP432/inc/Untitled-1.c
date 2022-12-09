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

#define qdo 262
#define qre 294
#define qmi 330 
#define qfa 349
#define qso 392
#define qla 440
#define qsi 494
#define do 523
#define re 578
#define mi 659
#define fa 698
#define so 784
#define la 880
#define si 988
#define do1 1046
#define re1 1175
#define mi1 1318
#define fa1 1480
#define so1 1568
#define la1 1760
#define si1 1976
#define sqdo 277
#define sqre 311
#define sqfa 370
#define sqso 415
#define sqla 466
#define sdo 554
#define sre 622
#define sfa 740
#define sso 831
#define sla 932
#define sdo1 1046
#define sre1 1245
#define sfa1 1480
#define sso1 1661
#define sla1 1865


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
    for (i=0;i<time * 2;i++)
    {
        P2->OUT |= 0x10;   // on Set P2.4...
        delay_us(delay);   //...for a semiperiod...
        P2->OUT &= ~0x10;  // off, reset it...
        delay_us(delay);   //...for the other semiperiod.
    }
    vTaskDelay(delay_beep / 2);
//    delay_ms(delay_beep); //Add a little delay to separate the single notes
}

// Function: play_song
// Description: This is the Imperial March code.
//              As you can see, there are lots of beeps at different frequencies and durations,
//              and some delays to separate the various bits of this wonderful song.
void play_song()
{
int pai=400,ban=200;
int ting=128;

Beep(la,ban);
Beep(si,ban);
vTaskDelay(128);
Beep(do1,pai+ban);
Beep(si,ban);
vTaskDelay(128);
Beep(do1,pai);
vTaskDelay(128);
Beep(mi1,pai);
vTaskDelay(ting);
Beep(si,3*pai);
vTaskDelay(ting);
Beep(mi,ban);
Beep(mi,ban);
Beep(la,ban+pai);
Beep(so,ban);
vTaskDelay(ting);
Beep(la,pai);
vTaskDelay(ting);
Beep(do1,pai);
vTaskDelay(ting);
Beep(so,2*pai);
vTaskDelay(ting);
vTaskDelay(pai);
Beep(mi,ban);
vTaskDelay(ting/2);
Beep(mi,ban);
vTaskDelay(ting/2);
Beep(fa,pai+ban);
Beep(mi,ban);
vTaskDelay(ting);
Beep(fa,ban);
Beep(do1,ban+pai);
vTaskDelay(ting);
Beep(mi,2*pai);
vTaskDelay(ting);
vTaskDelay(ban);
Beep(do1,ban);
vTaskDelay(ting/2);
Beep(do1,ban);
vTaskDelay(ting/2);
Beep(do1,ban);
vTaskDelay(ting/2);
Beep(si,ban+pai);
Beep(sfa,ban);
vTaskDelay(ting);
Beep(sfa,pai);
Beep(si,pai);
vTaskDelay(ting);
Beep(si,2*pai);
vTaskDelay(ting);
vTaskDelay(pai);
Beep(la,ban);
Beep(si,ban);
vTaskDelay(ting);
Beep(do1,pai+ban);
Beep(si,ban);
vTaskDelay(ting);
Beep(do1,pai);
vTaskDelay(ting);
Beep(mi1,pai);
vTaskDelay(ting);
Beep(si,2*pai);
vTaskDelay(ting);
vTaskDelay(pai);
Beep(mi,ban);
vTaskDelay(20);
Beep(mi,ban);
vTaskDelay(ting);
Beep(la,pai+ban);
Beep(so,ban);
vTaskDelay(ting);
Beep(la,pai);
vTaskDelay(ting);
Beep(do1,pai);
vTaskDelay(ting);
Beep(so,3*pai);
vTaskDelay(ting+ban);
Beep(mi,ban);
vTaskDelay(ting/2);
vTaskDelay(2000);
//    delay_ms(2000); //Add a 2 sec. delay to avoid replaying right after the end.

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
    for (i = 0; i<= us; i++)
       __delay_cycles(1);
}