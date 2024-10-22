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

/* Standard includes. */
#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* TI includes. */
#include "gpio.h"

/* ARM Cortex */
#include <stdint.h>
#include "msp.h"
#include "SysTick.h"
#include "inc/CortexM.h"



#include "./inc/songFile.h"
#include "./inc/dcMotor.h"
#include "./inc/bumpSwitch.h"
#include "./inc/outputLED.h"
#include "./inc/SysTick.h"
#define SW1IN ((*((volatile uint8_t *)(0x42098004)))^1) // input:switch SW1
#define SW2IN ((*((volatile uint8_t *)(0x42098010)))^1) // input: switch SW2


uint8_t bumpSwitch_status; // bump switch value
uint8_t song_en = 0; // song_en = 0: The Imperial March ; song_en = 1 Ode to Joy.
uint8_t mode = 0;

static SemaphoreHandle_t bin_sem;

// static void Switch_Init
static void Switch_Init(void){
    P1->SEL0 &= ~0x12;
    P1->SEL1 &= ~0x12;      // configure P1.4 and P1.1 as GPIO
    P1->DIR &= ~0x12;       // make P1.4 and P1.1 in
    P1->REN |= 0x12;        // enable pull resistors on P1.4 and P1.1
    P1->OUT |= 0x12;        // P1.4 and P1.1 are pull-up
};


static void taskMasterThread( void *pvParameters );
static void taskBumpSwitch(void *pvParameters);
static void taskPlaySong(void *pvParameters);
static void taskdcMotor(void *pvParameters);
static void taskReadInputSwitch(void *pvParameters);
static void taskdcMotor(void *pvParameters);
static void taskDisplayOutputLED(void *pvParameters);
static void taskStopDCmotor(void *pvParameters);
static void task_interrupt(void *pvParameters);
static void prvConfigureClocks( void );


xTaskHandle taskHandle_BlinkRedLED;
xTaskHandle taskHandle_BumpSwitch;
xTaskHandle taskHandle_PlaySong;
xTaskHandle taskHandle_dcMotor;
xTaskHandle taskHandle_InputSwitch;
xTaskHandle taskHandle_OutputLED;
xTaskHandle taskHandle_stopDC;
xTaskHandle taskHandle_intterupt;



void main_program(void);
void PORT4_IRQHandler(void);

void main_program( void )
{
    // initialise the clock configuration
    prvConfigureClocks();
    // TODO: initialise the switch
    Switch_Init();
    // TODO: initialise systick timer
    SysTick_Init();

    DisableInterrupts(); // used for IRQ


        xTaskCreate
        (
            taskMasterThread,
            "taskT",
            128,
            NULL,
            4,
            &taskHandle_BlinkRedLED
        );


        xTaskCreate
        (
            taskBumpSwitch,
            "taskB",
            128,
            NULL,
            2,
            &taskHandle_BumpSwitch
        );


        xTaskCreate
        (
            taskPlaySong,
            "taskS",
            128,
            NULL,
            2,
            &taskHandle_PlaySong
        );


        xTaskCreate
        (
            taskdcMotor,
            "taskM",
            128,
            NULL,
            2,
            &taskHandle_dcMotor
        );


        xTaskCreate
        (
            taskReadInputSwitch,
            "taskR",
            128,
            NULL,
            3,
            &taskHandle_InputSwitch
        );


        xTaskCreate
        (
            taskDisplayOutputLED,
            "taskD",
            128,
            NULL,
            2,
            &taskHandle_OutputLED
        );

         xTaskCreate
         (
            taskStopDCmotor,
            "stop_DC_motor",
            128,
            NULL,
            1,
            &taskHandle_stopDC
        );

        vTaskStartScheduler();

    /* INFO: If everything is fine, the scheduler will now be running,
    and the following line will never be reached.  If the following line
    does execute, then there was insufficient FreeRTOS heap memory
    available for the idle and/or timer tasks to be created. See the
    memory management section on the FreeRTOS web site for more details. */
    for( ;; );
}

//-----------------------------------------------------------------
// default state: taskStopDCmotor is in the lowest priority
// the function of it is to prevent the chaos of taskdcMotor.
// When the dcMotor is in 'running state', and the priority of dcMotor is changed. the conflict may occur in the system. the wheel will run with very fast speed.
// to avoid this situation, let taskStopDCmotor to stop motor at first, and then change the dc motor priority.
//-----------------------------------------------------------------
static void taskStopDCmotor(void *pvParameters){
    dcMotor_Init();
    while(1){
        dcMotor_Stop(1);
    };
};

//-------------------------------------------------------------------
// it is for the IRQ interrupt.
// unfortunately, we failed in creating IRQ interrupt system. So this function just play nothing.
//-------------------------------------------------------------------
static void task_interrupt(void *pvParameters){
    while(1){
        EnableInterrupts();
        xSemaphoreTake(bin_sem, portMAX_DELAY);
        dcMotor_response(bumpSwitch_status);
        dcMotor_Forward(300,1);
    }
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


static void Switch_Init(void );

//---------------------------------------------------------------------
// mode_LED:
// The LED is used to let user know which mode the system played.
// mode1.a: running wheel and play The Imperial March; The Blue LED on.
// mode1.b: Stop wheel and play The Ode to Joy;        The Pink LED on.
//---------------------------------------------------------------------
void mode_LED(char i_SW2){
    if(i_SW2 == 1){
        Port2_Output2(PINK);
    }
    else if(i_SW2 == 0){
        Port2_Output2(BLUE);
    }
}

//---------------------------------------------------------------------
// taskReadInputSwitch:
// in Mode 1:
//          switch 1: to control the song to be resumed or suspended.
//                    to control the priority based on the users choice. (mode1.a or mode1.b)
//
//          switch 2: to let users to choose the mode they want to play
//                    mode1.a: running wheel and play The Imperial March; The Blue LED on.
//                    mode1.b: Stop wheel and play The Ode to Joy;The Pink LED on.
// in Mode 2:
//          this mode is used to test the IRQ interrupt. However, the IRQ interrupt is failed.
//          By the way, the Mode 2 just play nothing.
//---------------------------------------------------------------------
static void taskReadInputSwitch(void *pvParameters){
    char i_SW1=0;char i_SW2=0;
    int i;
    while(1)
    {
        while(mode == 1){
            mode_LED(i_SW2);
            vTaskResume(taskHandle_BumpSwitch);

            if (SW1IN == 1 && SW2IN ==0)
            {
                i_SW1 ^= 1;                // when press switch 1, 1 => 0 ; 0 => 1;
                for (i=0; i<1000000; i++); // this waiting loop is used , to prevent the switch bounce.
            }

            if (SW2IN == 1 && SW1IN == 0)
            {
                i_SW2 ^= 1;                 // when press switch 2, 1 => 0 ; 0 => 1;
                for (i=0; i<1000000; i++); // this waiting loop is used , to prevent the switch bounce.
            }

            if (i_SW1 == 1)
            {
                REDLED = 1;     // turn on the red LED
                vTaskSuspend(taskHandle_PlaySong);//suspend the task taskHandle_PlaySong

                // the mode of system is in mode.1.a
                if(i_SW2 == 0)
                {
                    vTaskPrioritySet(NULL, 2);
                    vTaskPrioritySet(taskHandle_stopDC,1);
                    vTaskPrioritySet(taskHandle_PlaySong, 2);
                    song_en = 0;
                }

                // the mode of system is in mode.1.b
                else
                {
                    vTaskPrioritySet(taskHandle_stopDC,3);
                    vTaskPrioritySet(NULL, 3);
                    vTaskPrioritySet(taskHandle_PlaySong, 3);
                    song_en = 1;
                }

            }

            else if(i_SW1 == 0)
            {
                REDLED = 0;     // turn off the red LED
                vTaskResume(taskHandle_PlaySong);   // resume the task taskHandle_PlaySong
            }

        }

        while (mode == 2)
        {
            xTaskCreate(task_interrupt,'IRQ',128,NULL,3,&taskHandle_intterupt);
            mode_LED(i_SW2);

            if(i_SW2 == 0)
            {song_en = 0;}

            else
            {song_en = 1;}
        }
}
}
//---------------------------------------------------------------------------------------------------------
//TaskPlaySong:
// when user press switch 2 in mode 1, the system will add suitable song behind the current playing song.
//---------------------------------------------------------------------------------------------------------
static void taskPlaySong(void *pvParameters){
    // initialise the song
    init_song_pwm();
    // play the song's function and run forever
    while(1){
    // press the sw2 to change the song.
      if(song_en==1)
      {play_song_joy();}
      else
      {play_song();}
    }

}

//---------------------------------------------------------------------------------------------------------
// static void function for taskBumpSwitch
//---------------------------------------------------------------------------------------------------------
static void taskBumpSwitch(void *pvParameters)
{
    // initialise bump switches
    uint8_t i = 0;
    BumpSwitch_Init();
    Port1_Init2();

    //  Read the input of bump switches forever:
    while(1)
    {bumpSwitch_status = Bump_Read_Input();}
}

//---------------------------------------------------------------------------------------------------------
//static void function for taskDisplayOutputLED
//---------------------------------------------------------------------------------------------------------
static void taskDisplayOutputLED(void *pvParameters)
{
    // create a static void function for taskDisplayOutputLED
    RedLED_Init();
    while(1)
        {outputLED_response(bumpSwitch_status);}
}


//---------------------------------------------------------------------------------------------------------
//static void function for taskMasterThread
//  press switch 1: mode 1, and suspend taskMasterThread forever.
//  press switch 2: mode 2, and suspend taskMasterThread forever.
//---------------------------------------------------------------------------------------------------------
static void taskMasterThread( void *pvParameters )
{
    int i;
    //initialise the color LED and red LED
    ColorLED_Init();
    RedLED_Init();

    while(!SW2IN && !SW1IN)
    {
        for (i=0; i<1000000; i++);
        REDLED = !REDLED;
    }

    // if press sw1, get into mode1: task with dynamic priority scheduling.
    // bump switch detection: polling
    while(!SW2IN && SW1IN)
    {
        REDLED = 0;
        mode = 1;
        vTaskSuspend(taskHandle_BlinkRedLED);
    }

    // if press sw2, get into mode2
    // bump switch detection: polling
    while(SW2IN && !SW1IN)
    {
        REDLED = 0;
        mode = 2;
        vTaskSuspend(taskHandle_BlinkRedLED);
    }
}

//---------------------------------------------------------------------------------------------------------
//static void function for taskdcMotor
//      use polling function to detect bumpSwitch, and rotate in suitable direction.
//---------------------------------------------------------------------------------------------------------
static void taskdcMotor(void *pvParameters){
    //initialise the DC Motor
    dcMotor_Init();
    //  use a polling that continuously read from the bumpSwitch_status,
    while(1)
    {
        if (mode == 1 && (bumpSwitch_status == 0x6D || bumpSwitch_status == 0xAD || bumpSwitch_status == 0xCD || bumpSwitch_status == 0xE5 || bumpSwitch_status == 0xE9 || bumpSwitch_status == 0xEC))
        {
            dcMotor_response(bumpSwitch_status);
        }

        dcMotor_Forward(500,1);
    }
}

//---------------------------------------------------------------------------------------------------------
//static void function for PORT4_IRQHandler
//      ideal situation: the PORT4_IRQHandler would detect the bump switch all the time, like taskdcMotor(POLLING).
//      However, the interrupt detection system is failed.
//---------------------------------------------------------------------------------------------------------
void PORT4_IRQHandler(void){
    bumpSwitch_status = P4->IV;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    P4->IFG &= ~0xED;
    xSemaphoreGiveFromISR(bin_sem,&xHigherPriorityTaskWoken);
}
