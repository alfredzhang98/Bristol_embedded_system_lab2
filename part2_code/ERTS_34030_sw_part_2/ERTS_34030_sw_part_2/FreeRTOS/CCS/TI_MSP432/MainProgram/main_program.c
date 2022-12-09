/*
 * FreeRTOS Kernel V10.1.1
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

// Author:      Mohd A. Zainol
// Date:        1 Oct 2018
// Chip:        MSP432P401R LaunchPad Development Kit (MSP-EXP432P401R) for TI-RSLK
// File:        main_program.c
// Function:    The main function of our code in FreeRTOS

#define DEFAULT 0x00
#define POLLING_MDOE 0x01
#define INTERRUPT_MODE 0x02

#define HIGH_PRIORITY 4
#define MID_PRIORITY 3
#define LOW_PRIORITY 2
#define LOW_LOW_PRIORITY 1

/* Standard includes. */
#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* TI includes. */
#include "gpio.h"

/* ARM Cortex */
#include <stdint.h>
#include "msp.h"
#include "SysTick.h"
#include "inc/CortexM.h"

// TODO: include the songFile header file
#include "./inc/songFile.h"
// TODO: include the dcMotor header file
#include "./inc/dcMotor.h"
// TODO: include the bumpSwitch header file
#include "./inc/bumpSwitch.h"
// TODO: include the outputLED header file
#include "./inc/outputLED.h"
// TODO: include the SysTick header file
#include "./inc/SysTick.h"
// TODO: bit-banded addresses positive logic of input switch S1
#define SW1IN ((*((volatile uint8_t *)(0x42098004)))^1)
// TODO: bit-banded addresses positive logic of input switch S2
#define SW2IN ((*((volatile uint8_t *)(0x42098010)))^1)
//REDLED
#define REDLED (*((volatile uint8_t *)(0x42098040)))


// TODO: declare a global variable to read bump switches value,
//       name this as bumpSwitch_status and use uint8_t
// volatile data protect
volatile uint8_t bumpSwitch_status;
uint8_t play_status = 0;
uint8_t master_status = 1;
//use for pwm timer cnt
uint16_t cnt_motor;
//use for songs switch
uint16_t cnt_songs;

// function for timer
void vTimerCallback_bumpStatus(TimerHandle_t xTimer);
void vTimerCallback_Motor(TimerHandle_t xTimer);


// static void Switch_Init
static void Switch_Init(void);

// a static void function for a task called "taskMasterThread"
static void taskMasterThread( void *pvParameters );

// TODO: declare a static void function for a task called "taskBumpSwitch"
void static taskBumpSwitch();
// TODO: declare a static void function for a task called "taskPlaySong"
void static taskPlaySong();
// TODO: declare a static void function for a task called "taskdcMotor"
void static taskdcMotor();
// TODO: declare a static void function for a task called "taskReadInputSwitch"
void static taskReadInputSwitch(void *pvParameters);
// TODO: declare a static void function for a task called "taskdcMotor"
//void static taskdcMotor();
//  TODO: declare a static void function for a task called "taskDisplayOutputLED"
void static taskDisplayOutputLED();
//IRQ
void static taskIRQ();
//void static taskdcMotor();
/*
 * Called by main() to create the main program application
 */
void main_program( void );

/*
 * The configuration of clocks for frequency.
 */
static void prvConfigureClocks( void );

//TIMER handler status
TimerHandle_t xTimer_getBumpStatus;
//TIMER handler motor normal movement
TimerHandle_t xTimer_Motor;

// declare an identifier of task handler called "taskHandle_BlinkRedLED"
xTaskHandle taskHandle_BlinkRedLED = NULL;
// TODO: declare an identifier of task handler called "taskHandle_BumpSwitch"
xTaskHandle taskHandle_BumpSwitch = NULL;
// TODO: declare an identifier of task handler called "taskHandle_PlaySong"
xTaskHandle taskHandle_PlaySong = NULL;
// TODO: declare an identifier of task handler called "taskHandle_dcMotor"
xTaskHandle taskHandle_dcMotor = NULL;
// TODO: declare an identifier of task handler called "taskHandle_InputSwitch"
xTaskHandle taskHandle_InputSwitch = NULL;
// TODO: declare an identifier of task handler called "taskHandle_OutputLED"
xTaskHandle taskHandle_OutputLED = NULL;
// IRQ
xTaskHandle taskHandle_IRQ = NULL;


void main_program( void )
{

//#define DEFAULT 0x00
//#define POLLING_MDOE 0x01
//#define INTERRUPT_MODE 0x02
    // initialise the clock configuration
    prvConfigureClocks();

    //LED
    RedLED_Init();
    ColorLED_Init();

    // TODO: initialise the switch
    Switch_Init();
    //if we do not use the taskBumpSwitch task we should init the bump switch at first
    BumpSwitch_Init();

    // TODO: initialise systick timer
    SysTick_Init();

    //////////////////////////////////////////////////////
    // TIP: to create a task, use xTaskCreate in FreeRTOS
    // URL : https://www.freertos.org/a00125.html
    //////////////////////////////////////////////////////
        //timer
        xTimer_getBumpStatus = xTimerCreate("Timer_bumpStatus", 100, pdTRUE, ( void * ) 1, vTimerCallback_bumpStatus);
        xTimerStart(xTimer_getBumpStatus,200);
        xTimer_Motor = xTimerCreate("Timer_motor", 10, pdTRUE, ( void * ) 1, vTimerCallback_Motor);
        xTimerStart(xTimer_Motor,200);

        // TODO: Create a task that has these parameters=
        //       pvTaskCode: taskMasterThread
        //       pcName: taskT
        //       usStackDepth: 128
        //       pvParameters: NULL
        //       uxPriority: 2
        //       pxCreatedTask: taskHandle_BlinkRedLED
        xTaskCreate(taskMasterThread, "taskT", 128, NULL, HIGH_PRIORITY, &taskHandle_BlinkRedLED);

        // TODO: Create a task that has these parameters=
        //       pvTaskCode: taskBumpSwitch
        //       pcName: taskB
        //       usStackDepth: 128
        //       pvParameters: NULL
        //       uxPriority: 1
        //       pxCreatedTask: taskHandle_BumpSwitch
//        xTaskCreate(taskBumpSwitch, "taskB", 128, NULL, LOW_PRIORITY, &taskHandle_BumpSwitch);

        // TODO: Create a task that has these parameters=
        //       pvTaskCode: taskPlaySong
        //       pcName: taskS
        //       usStackDepth: 128
        //       pvParameters: NULL
        //       uxPriority: 1
        //       pxCreatedTask: taskHandle_PlaySong //MID_PRIORITY
        xTaskCreate(taskPlaySong, "taskS", 128, NULL, MID_PRIORITY, &taskHandle_PlaySong);

        // TODO: Create a task that has these parameters=
        //       pvTaskCode: taskdcMotor
        //       pcName: taskM
        //       usStackDepth: 128
        //       pvParameters: NULL
        //       uxPriority: 1
        //       pxCreatedTask: taskHandle_dcMotor
        xTaskCreate(taskdcMotor, "taskM", 128, NULL, LOW_PRIORITY, &taskHandle_dcMotor);

        // TODO: Create a task that has these parameters=
        //       pvTaskCode: taskReadInputSwitch
        //       pcName: taskR
        //       usStackDepth: 128
        //       pvParameters: NULL
        //       uxPriority: 1
        //       pxCreatedTask: taskHandle_InputSwitch
        xTaskCreate(taskReadInputSwitch, "taskR", 128, NULL, LOW_PRIORITY, &taskHandle_InputSwitch);

        // TODO: Create a task that has these parameters=
        //       pvTaskCode: taskDisplayOutputLED
        //       pcName: taskD
        //       usStackDepth: 128
        //       pvParameters: NULL
        //       uxPriority: 1
        //       pxCreatedTask: taskHandle_OutputLED
        xTaskCreate(taskDisplayOutputLED, "taskD", 128, NULL, LOW_PRIORITY, &taskHandle_OutputLED);

        //IRQ
        xTaskCreate(taskIRQ, "taskIRQ", 128, NULL, LOW_LOW_PRIORITY, &taskHandle_IRQ);

        //////////////////////////////////////////////////////////////////
        // TIP: to start a scheduler, use vTaskStartScheduler in FreeRTOS
        // URL : https://www.freertos.org/a00132.html
        //////////////////////////////////////////////////////////////////

        // TODO: start the scheduler
        vTaskStartScheduler();

    /* INFO: If everything is fine, the scheduler will now be running,
    and the following line will never be reached.  If the following line
    does execute, then there was insufficient FreeRTOS heap memory
    available for the idle and/or timer tasks to be created. See the
    memory management section on the FreeRTOS web site for more details. */
    for( ;; );
}
/*-----------------------------------------------------------------*/
/*------------------- FreeRTOS configuration ----------------------*/
/*-------------   DO NOT MODIFY ANYTHING BELOW HERE   -------------*/
/*-----------------------------------------------------------------*/
// The configuration clock to be used for the board
static void prvConfigureClocks( void )
{
    // Set Flash wait state for high clock frequency
    FlashCtl_setWaitState( FLASH_BANK0, 2 );
    FlashCtl_setWaitState( FLASH_BANK1, 2 );

    // This clock configuration uses maximum frequency.
    // Maximum frequency also needs more voltage.

    // From the datasheet: For AM_LDO_VCORE1 and AM_DCDC_VCORE1 modes,
    // the maximum CPU operating frequency is 48 MHz
    // and maximum input clock frequency for peripherals is 24 MHz.
    PCM_setCoreVoltageLevel( PCM_VCORE1 );
    CS_setDCOCenteredFrequency( CS_DCO_FREQUENCY_48 );
    CS_initClockSignal( CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1 );
    CS_initClockSignal( CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1 );
    CS_initClockSignal( CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1 );
    CS_initClockSignal( CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1 );
}

// The sleep processing for MSP432 board
void vPreSleepProcessing( uint32_t ulExpectedIdleTime ){}

#if( configCREATE_SIMPLE_TICKLESS_DEMO == 1 )
    void vApplicationTickHook( void )
    {
        /* This function will be called by each tick interrupt if
        configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
        added here, but the tick hook is called from an interrupt context, so
        code must not attempt to block, and only the interrupt safe FreeRTOS API
        functions can be used (those that end in FromISR()). */
        /* Only the full demo uses the tick hook so there is no code is
        executed here. */
    }
#endif
/*-----------------------------------------------------------------*/
/*-------------   DO NOT MODIFY ANYTHING ABOVE HERE   -------------*/
/*--------------------------- END ---------------------------------*/
/*-----------------------------------------------------------------*/

static void Switch_Init(void){
    // negative logic built-in Button 1 connected to P1.1
    // negative logic built-in Button 2 connected to P1.4
    P1->SEL0 &= ~0x12;
    P1->SEL1 &= ~0x12;      // configure P1.4 and P1.1 as GPIO
    P1->DIR &= ~0x12;       // make P1.4 and P1.1 in
    P1->REN |= 0x12;        // enable pull resistors on P1.4 and P1.1
    P1->OUT |= 0x12;        // P1.4 and P1.1 are pull-up
}
// a static void function for taskReadInputSwitch
static void taskReadInputSwitch( void *pvParameters ){

    char i_SW1=0;
    int i;

    for( ;; )
    {
        vTaskDelay(10);
        if (SW1IN == 1) {
            i_SW1 ^= 1;                 // toggle the variable i_SW1
            for (i=0; i<1000000; i++);  // this waiting loop is used
                                        // to prevent the switch bounce.
        }
        if (SW2IN == 1) {
            REDLED = 1;
            play_status ^= 1;
            for (i=0; i<1000000; i++);  // this waiting loop is used
                                        // to prevent the switch bounce.
            REDLED = 0;
        }

        ///////////////////////////////////////////////////////////
        // TIP: to suspend a task, use vTaskSuspend in FreeRTOS
        // URL: https://www.freertos.org/a00130.html
        ///////////////////////////////////////////////////////////
//        vTaskSuspend(pvParameters);
//        vTaskSuspend(NULL);
        ///////////////////////////////////////////////////////////
        // TIP: to resume a task, use vTaskResume in FreeRTOS
        // URL: https://www.freertos.org/a00131.html
        ///////////////////////////////////////////////////////////
//        vTaskResume(pvParameters);
//        vTaskResume(NULL);

        if (i_SW1 == 1) {
            //REDLED = 1;     // turn on the red LED
            // TODO: suspend the task taskHandle_PlaySong
            if(taskHandle_PlaySong != NULL){
                vTaskSuspend(taskHandle_PlaySong);
            }

        }
        else {
            //REDLED = 0;     // turn off the red LED
            // TODO: resume the task taskHandle_PlaySong
            if(taskHandle_PlaySong != NULL){
                vTaskResume(taskHandle_PlaySong);
            }
        }
    }
}

// TODO: create a static void function for taskPlaySong
static void taskPlaySong(){
    // TODO: initialise the song
    init_song_pwm();

    // TODO: play the song's function and run forever
    while(1){
        if(play_status == 0) play_song();
        if(play_status == 1) play_song_1();
    }
}

// TODO: create a static void function for taskBumpSwitch
static void taskBumpSwitch(){
    // TODO: initialise bump switches
    BumpSwitch_Init();
    // TODO: Read the input of bump switches forever:
    //       Continuously read the 6 bump switches in a loop,
    //       and return it to the "bumpSwitch_status" variable.
    //       Note that the bumpSwitch_status is a global variable,
    //       so do not declare it again here locally.
    for( ;; ) // uncomment this
    { // uncomment this

//         TODO: use bumpSwitch_status as the variable and
//               use Bump_Read_Input to read the input
        bumpSwitch_status = Bump_Read_Input();

        if(bumpSwitch_status != 0xED){
            vTaskPrioritySet(taskHandle_OutputLED, MID_PRIORITY);
            vTaskPrioritySet(taskHandle_dcMotor, MID_PRIORITY);
        }
        vTaskDelay(10);
    } // uncomment this

}
// TODO: create a static void function for taskDisplayOutputLED
static void taskDisplayOutputLED(){
    for( ;; ) // uncomment this
    { // uncomment this

        // TODO: use outputLED_response as the function and
        //       use bumpSwitch_status as the parameter
        outputLED_response(bumpSwitch_status);
        vTaskPrioritySet(taskHandle_OutputLED, LOW_PRIORITY);

    } // uncomment this
}
// a static void function for taskMasterThread
static void taskMasterThread( void *pvParameters )
{
    int i;

    // TODO: initialise the color LED
    Port2_Output2(WHITE);

    // initialise the red LED
//    RedLED_Init();

    while(!SW2IN);
    // TODO: Turn off the RED LED, we no longer need that.
    REDLED = 0;

    //////////////////////////////////////////////////////////////////
    // TIP: to suspend a task, use vTaskSuspend in FreeRTOS
    // URL: https://www.freertos.org/a00130.html
    //////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////
    // TIP: to delete a task, use vTaskDelete in FreeRTOS
    // URL: https://www.freertos.org/a00126.html
    //////////////////////////////////////////////////////////////////

    // TODO: This function (taskMasterThread)is no longer needed.
    //       Please suspend this task itself, or maybe just delete it.
    //       Question: what are the difference between 'suspend' the task,
    //                 or 'delete' the task?
//    vTaskSuspend(NULL);
//    vTaskDelete(NULL);
    if( taskHandle_BlinkRedLED != NULL )
    {
//        vTaskSuspend(taskHandle_BlinkRedLED);
        vTaskDelete(taskHandle_BlinkRedLED);
        master_status = 0;
        Port2_Output2(0);
    }else{
        Port2_Output2(RED);
    }
}

// TODO: create a static void function for taskdcMotor
static void taskdcMotor(){
    // TODO: initialise the DC Motor
    dcMotor_Init();
    // TODO: use a polling that continuously read from the bumpSwitch_status,
    //       and run this forever in a while loop.
    //       use dcMotor_response and bumpSwitch_status for the parameter
    while(1){
        //check the bump status
        if(bumpSwitch_status != 0xED){
            //stop the normal movement to response to the bump
            xTimerStop(xTimer_Motor,200);
        }
        dcMotor_response(bumpSwitch_status);
        vTaskDelay(10);
        //reset the timer for the motor
        xTimerReset(xTimer_Motor,200);
        vTaskPrioritySet(taskHandle_dcMotor, LOW_PRIORITY);
        vTaskPrioritySet(taskHandle_IRQ, LOW_LOW_PRIORITY);
    }
}

static void taskIRQ(){

    while(1){                  // Wait for SW2 switch
        vTaskDelay(200);
        REDLED = !REDLED;           // The red LED is blinking
    }

}

//callback check the bump status every 100ms
void vTimerCallback_bumpStatus( TimerHandle_t xTimer )
{
    bumpSwitch_status = Bump_Read_Input();
    if(bumpSwitch_status != 0xED){
        vTaskPrioritySet(taskHandle_OutputLED, MID_PRIORITY);
        vTaskPrioritySet(taskHandle_dcMotor, MID_PRIORITY);
        vTaskPrioritySet(taskHandle_IRQ, MID_PRIORITY);
    }
}

//callback the normal forward moving to realised the PWM movement
void vTimerCallback_Motor( TimerHandle_t xTimer )
{
    cnt_motor++;
    if(cnt_motor == 0xFFFF) cnt_motor = 0;
    if(cnt_motor%2 == 0){
        P2->OUT |= 0xC0;   // on
    }else{
        P2->OUT &= ~0xC0;  // off
    }

}

