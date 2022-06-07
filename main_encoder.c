/*
 * FreeRTOS V202112.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/******************************************************************************
 * NOTE 1: The FreeRTOS demo threads will not be running continuously, so
 * do not expect to get real time behaviour from the FreeRTOS Linux port, or
 * this demo application.  Also, the timing information in the FreeRTOS+Trace
 * logs have no meaningful units.  See the documentation page for the Linux
 * port for further information:
 * https://freertos.org/FreeRTOS-simulator-for-Linux.html
 *
 * NOTE 2:  This project provides two demo applications.  A simple blinky style
 * project, and a more comprehensive test and demo application.  The
 * mainCREATE_SIMPLE_BLINKY_DEMO_ONLY setting in main.c is used to select
 * between the two.  See the notes on using mainCREATE_SIMPLE_BLINKY_DEMO_ONLY
 * in main.c.  This file implements the simply blinky version.  Console output
 * is used in place of the normal LED toggling.
 *
 * NOTE 3:  This file only contains the source code that is specific to the
 * basic demo.  Generic functions, such FreeRTOS hook functions, are defined
 * in main.c.
 ******************************************************************************
 *
 * main_blinky() creates one queue, one software timer, and two tasks.  It then
 * starts the scheduler.
 *
 * The Queue Send Task:
 * The queue send task is implemented by the prvQueueSendTask() function in
 * this file.  It uses vTaskDelayUntil() to create a periodic task that sends
 * the value 100 to the queue every 200 milliseconds (please read the notes
 * above regarding the accuracy of timing under Linux).
 *
 * The Queue Send Software Timer:
 * The timer is an auto-reload timer with a period of two seconds.  The timer's
 * callback function writes the value 200 to the queue.  The callback function
 * is implemented by prvQueueSendTimerCallback() within this file.
 *
 * The Queue Receive Task:
 * The queue receive task is implemented by the prvQueueReceiveTask() function
 * in this file.  prvQueueReceiveTask() waits for data to arrive on the queue.
 * When data is received, the task checks the value of the data, then outputs a
 * message to indicate if the data came from the queue send task or the queue
 * send software timer.
 *
 * Expected Behaviour:
 * - The queue send task writes to the queue every 200ms, so every 200ms the
 *   queue receive task will output a message indicating that data was received
 *   on the queue from the queue send task.
 * - The queue send software timer has a period of two seconds, and is reset
 *   each time a key is pressed.  So if two seconds expire without a key being
 *   pressed then the queue receive task will output a message indicating that
 *   data was received on the queue from the queue send software timer.
 *
 * NOTE:  Console input and output relies on Linux system calls, which can
 * interfere with the execution of the FreeRTOS Linux port. This demo only
 * uses Linux system call occasionally. Heavier use of Linux system calls
 * may crash the port.
 */

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>

/* Kernel includes. */
#include "FreeRTOS/Source/include/FreeRTOS.h"
#include "FreeRTOS/Source/include/task.h"
#include "FreeRTOS/Source/include/timers.h"
#include "FreeRTOS/Source/include/semphr.h"

/* Local includes. */

/* Priorities at which the tasks are created. */
#define mainENCODER_PRIORITY     ( tskIDLE_PRIORITY + 2 )
#define mainTASK_PRIORITY        ( tskIDLE_PRIORITY + 3 )
#define mainSCOPE_PRIORITY		 ( tskIDLE_PRIORITY + 1 )



/* The rate at which data is sent to the queue.  The times are converted from
 * milliseconds to ticks using the pdMS_TO_TICKS() macro. */
#define BASE_PERIOD_MS 5
#define mainENCODER_TICK_FREQUENCY			pdMS_TO_TICKS( BASE_PERIOD_MS )
#define mainTASK1_TICK_FREQUENCY			pdMS_TO_TICKS( BASE_PERIOD_MS/2 )
#define mainTASK2_TICK_FREQUENCY			pdMS_TO_TICKS( BASE_PERIOD_MS/2 )
#define mainSCOPE_TICK_FREQUENCY			pdMS_TO_TICKS( BASE_PERIOD_MS*2 )

/*-----------------------------------------------------------*/

/*
 * The tasks as described in the comments at the top of this file.
 */
static void encoderTask( void * pvParameters );
static void rt1Task( void * pvParameters );
static void rt2Task( void * pvParameters );
static void scopeTask( void * pvParameters );

/*-----------------------------------------------------------*/

struct enc_str
{
	unsigned int slit;		//valori oscillanti tra 0 e 1
	unsigned int home_slit;	//1 se in home, 0 altrimenti
	SemaphoreHandle_t lock;
};
static struct enc_str enc_data;

struct _rising_edge{
	unsigned int count;
	SemaphoreHandle_t lock;
};
static struct _rising_edge rising_edge;

struct _round_time{
	unsigned long int time_diff;
	SemaphoreHandle_t lock;
};
static struct _round_time round_time;

/*-----------------------------------------------------------*/

/*** SEE THE COMMENTS AT THE TOP OF THIS FILE ***/
void main_encoder( void )
{

    enc_data.lock = xSemaphoreCreateMutex();
    rising_edge.lock = xSemaphoreCreateMutex();
    round_time.lock = xSemaphoreCreateMutex();
    if (enc_data.lock != NULL &&  rising_edge.lock != NULL && round_time.lock != NULL)
    {
        xTaskCreate(encoderTask, "ENC", configMINIMAL_STACK_SIZE, NULL, mainENCODER_PRIORITY, NULL);
        xTaskCreate(rt1Task, "RT1", configMINIMAL_STACK_SIZE, NULL, mainTASK_PRIORITY, NULL);
        xTaskCreate(rt2Task, "RT2", configMINIMAL_STACK_SIZE, NULL, mainTASK_PRIORITY, NULL);
        xTaskCreate(scopeTask, "SCOPE", configMINIMAL_STACK_SIZE, NULL, mainSCOPE_PRIORITY, NULL);
        vTaskStartScheduler();
    }

    for( ; ; )
    {
    }
}

/*-----------------------------------------------------------*/

static void encoderTask(void * pvParameters){
    TickType_t xNextWakeTime;
    const TickType_t xBlockTime = mainENCODER_TICK_FREQUENCY;
    /* Prevent the compiler warning about the unused parameter. */
    ( void ) pvParameters;
    xNextWakeTime = xTaskGetTickCount();
    xSemaphoreTake(enc_data.lock, portMAX_DELAY);
    enc_data.slit = 0;
    enc_data.home_slit = 0;
    xSemaphoreGive(enc_data.lock);

    unsigned int count = 0;
    unsigned int slit_count = 0;
    unsigned int prev_slit = 0;

    /* Randomized period (75-750 RPM) */
	srand(time(NULL));
	unsigned int semi_per = (rand() % 10) + 1;

    for (;;)
    {
        vTaskDelayUntil( &xNextWakeTime, xBlockTime );
        xSemaphoreTake(enc_data.lock, portMAX_DELAY);
        prev_slit = enc_data.slit;
		if (count%semi_per == 0) {
			enc_data.slit++;
			enc_data.slit%=2;
		}

		if (prev_slit==0&&enc_data.slit==1) 					//fronte di salita
			slit_count=(++slit_count)%8;

		if (slit_count==0) enc_data.home_slit=enc_data.slit;
		else enc_data.home_slit=0;

		//printf("%d:\t\t %d %d\n",count,enc_data.slit,enc_data.home_slit);	//DEBUG encoder
		count++;
        xSemaphoreGive(enc_data.lock);

    }
    
}

/*-----------------------------------------------------------*/

static void rt1Task(void * pvParameters){
    TickType_t xNextWakeTime;
    const TickType_t xBlockTime = mainTASK1_TICK_FREQUENCY;
    /* Prevent the compiler warning about the unused parameter. */
    ( void ) pvParameters;
    xNextWakeTime = xTaskGetTickCount();

    xSemaphoreTake(rising_edge.lock, portMAX_DELAY);
    rising_edge.count = 0;
    xSemaphoreGive(rising_edge.lock);

    int last_value = 0;

    for (;;)
    {
        vTaskDelayUntil( &xNextWakeTime, xBlockTime );

        xSemaphoreTake(enc_data.lock, portMAX_DELAY);
        if( last_value == 0 && enc_data.slit == 1){
			last_value = 1;
			
        xSemaphoreTake(rising_edge.lock, portMAX_DELAY);
		rising_edge.count++;
		xSemaphoreGive(rising_edge.lock);
			
		}
		else if(last_value == 1 && enc_data.slit == 0){
			last_value = 0;
		}
        xSemaphoreGive(enc_data.lock);

    }
    
}
/*-----------------------------------------------------------*/

static void rt2Task(void * pvParameters){
    TickType_t xNextWakeTime;
    const TickType_t xBlockTime = mainTASK2_TICK_FREQUENCY;
    /* Prevent the compiler warning about the unused parameter. */
    ( void ) pvParameters;
    xNextWakeTime = xTaskGetTickCount();
	
    //TODO:AGGIORNARE
    TickType_t time_home;
	TickType_t last_time_home;
	
	int first_measure = 1;
	int last_home_slit = 0;

    for (;;)
    {
        vTaskDelayUntil( &xNextWakeTime, xBlockTime );
        xSemaphoreTake(enc_data.lock, portMAX_DELAY);
        if(enc_data.home_slit == 1 && last_home_slit == 0){
			last_home_slit = 1;
			if(first_measure){
				last_home_slit = xTaskGetTickCount();
				first_measure = 0;
			}
			else{	
				//clock_gettime(CLOCK_REALTIME, &time_home);
				time_home = xTaskGetTickCount();
                xSemaphoreTake(round_time.lock, portMAX_DELAY);
				round_time.time_diff = configTICK_RATE_HZ*(time_home - last_time_home);
                xSemaphoreGive(round_time.lock);
				last_time_home = time_home;	
			}
		}
		else if(enc_data.home_slit == 0){
			last_home_slit = 0;
		}
        xSemaphoreGive(enc_data.lock);
    
    }
    
}
/*-----------------------------------------------------------*/

static void scopeTask(void * pvParameters){
    TickType_t xNextWakeTime;
    const TickType_t xBlockTime = mainTASK2_TICK_FREQUENCY;
    /* Prevent the compiler warning about the unused parameter. */
    ( void ) pvParameters;
    xNextWakeTime = xTaskGetTickCount();
    unsigned int count=0;
	float diff_us = 0;
	unsigned int rpm = 0;

    for (;;)
    {
        vTaskDelayUntil( &xNextWakeTime, xBlockTime );

        xSemaphoreTake(rising_edge.lock, portMAX_DELAY);
        count = rising_edge.count;
        xSemaphoreGive(rising_edge.lock);

        printf("Rising Edge Counter : %d\t",count);

        xSemaphoreTake(round_time.lock, portMAX_DELAY);
        diff_us = round_time.time_diff/1000;
        xSemaphoreGive(round_time.lock);

        rpm = (unsigned int)((float)60*1000/diff_us);
        
        printf("RPM : %u\n",rpm);

    }
    
}
