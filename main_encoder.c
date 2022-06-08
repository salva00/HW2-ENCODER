/*AUTORI

    Anna Amodio             N46005101
    Federica Benevento      N46005192
    Salvatore Bramante      N46005183
    Alessia Manna           N46006759

*/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
//#include <conio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* Priorities at which the tasks are created. */
#define mainENCODER_PRIORITY     ( tskIDLE_PRIORITY + 2 )
#define mainTASK1_PRIORITY       ( tskIDLE_PRIORITY + 3 )
#define mainTASK2_PRIORITY       ( tskIDLE_PRIORITY + 3 )
#define mainSCOPE_PRIORITY		 ( tskIDLE_PRIORITY + 1 )
#define mainDIAG_PRIORITY        ( tskIDLE_PRIORITY + 1 )



/* The rate at which data is sent to the queue.  The times are converted from
 * milliseconds to ticks using the pdMS_TO_TICKS() macro. */
#define BASE_PERIOD_MS 5
#define mainENCODER_TICK_FREQUENCY			pdMS_TO_TICKS( BASE_PERIOD_MS )
#define mainTASK1_TICK_FREQUENCY			pdMS_TO_TICKS( BASE_PERIOD_MS/2 )
#define mainTASK2_TICK_FREQUENCY			pdMS_TO_TICKS( BASE_PERIOD_MS/2 )
#define mainSCOPE_TICK_FREQUENCY			pdMS_TO_TICKS( BASE_PERIOD_MS*2 )
#define mainDIAG_TICK_FREQUENCY             pdMS_TO_TICKS( BASE_PERIOD_MS*2 )     

/*-----------------------------------------------------------*/

/* The tasks */

static void encoderTask( void * pvParameters );
static void rt1Task( void * pvParameters );
static void rt2Task( void * pvParameters );
static void scopeTask( void * pvParameters );
static void diagnosticTask( void* pvParameters );

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

struct _slack_rt{
	unsigned long int slack_time;
    SemaphoreHandle_t lock;
};

static struct _slack_rt slack_rt1;
static struct _slack_rt slack_rt2;


/*-----------------------------------------------------------*/

void main_encoder( void )
{

    enc_data.lock = xSemaphoreCreateMutex();
    rising_edge.lock = xSemaphoreCreateMutex();
    round_time.lock = xSemaphoreCreateMutex();
    
    /*Creazione dei mutex per il calcolo dello slack time*/
    slack_rt1.lock = xSemaphoreCreateMutex();
    slack_rt2.lock = xSemaphoreCreateMutex();
    
    if (enc_data.lock != NULL &&  rising_edge.lock != NULL && round_time.lock != NULL)
    {
        xTaskCreate(encoderTask, "ENC", configMINIMAL_STACK_SIZE, NULL, mainENCODER_PRIORITY, NULL);
        xTaskCreate(rt1Task, "RT1", configMINIMAL_STACK_SIZE, NULL, mainTASK1_PRIORITY, NULL);
        xTaskCreate(rt2Task, "RT2", configMINIMAL_STACK_SIZE, NULL, mainTASK2_PRIORITY, NULL);
        xTaskCreate(scopeTask, "SCOPE", configMINIMAL_STACK_SIZE, NULL, mainSCOPE_PRIORITY, NULL);
        xTaskCreate(diagnosticTask, "DIAG", configMINIMAL_STACK_SIZE, NULL, mainDIAG_PRIORITY, NULL);
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

    /*per lo slack time*/
    TickType_t xFinishTime;

    xSemaphoreTake(rising_edge.lock, portMAX_DELAY);
    rising_edge.count = 0;
    xSemaphoreGive(rising_edge.lock);

    int last_value = 0;

    unsigned long int slack;

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

       
        xFinishTime = xTaskGetTickCount();
        slack = (xBlockTime + xNextWakeTime - xFinishTime) * portTICK_PERIOD_MS  ;
        if(slack>=0){
            xSemaphoreTake(slack_rt1.lock, portMAX_DELAY);
            slack_rt1.slack_time = slack;
            xSemaphoreGive(slack_rt1.lock);
        }
        else{
            printf("Task rt1: deadline miss. deadline: %lu, finish time: %lu\n",xNextWakeTime + xBlockTime ,xFinishTime);
        }
    
    }
}
/*-----------------------------------------------------------*/

static void rt2Task(void * pvParameters){
    TickType_t xNextWakeTime;
    const TickType_t xBlockTime = mainTASK2_TICK_FREQUENCY;
    /* Prevent the compiler warning about the unused parameter. */
    ( void ) pvParameters;
    xNextWakeTime = xTaskGetTickCount();
	
   
    TickType_t time_home;
	TickType_t last_time_home;
	
	int first_measure = 1;
	int last_home_slit = 0;

    TickType_t xFinishTime;
    unsigned long int slack;

    for (;;)
    {
        vTaskDelayUntil( &xNextWakeTime, xBlockTime );
        xSemaphoreTake(enc_data.lock, portMAX_DELAY);
        if(enc_data.home_slit == 1 && last_home_slit == 0){
			last_home_slit = 1;
			if(first_measure){
				last_time_home = xTaskGetTickCount();
				first_measure = 0;
			}
			else{	
				time_home = xTaskGetTickCount();
                xSemaphoreTake(round_time.lock, portMAX_DELAY);
				round_time.time_diff = portTICK_PERIOD_MS*(time_home - last_time_home);
                xSemaphoreGive(round_time.lock);
				last_time_home = time_home;	
			}
		}
		else if(enc_data.home_slit == 0){
			last_home_slit = 0;
		}
        xSemaphoreGive(enc_data.lock);
        
        
        xFinishTime = xTaskGetTickCount();
        slack = (xBlockTime + xNextWakeTime - xFinishTime)*portTICK_PERIOD_MS;
        if(slack >= 0){
            xSemaphoreTake(slack_rt2.lock, portMAX_DELAY);
            slack_rt2.slack_time = slack;
            xSemaphoreGive(slack_rt2.lock);
        }
        else{
            printf("Task rt2: deadline: %lu, finish time: %lu\n",xNextWakeTime + xBlockTime ,xFinishTime);
        }
    
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
        diff_us = round_time.time_diff * 1000;        //difference in microseconds
        xSemaphoreGive(round_time.lock);

        rpm = (unsigned int)((float)60*1000000/diff_us);
        
        //printf("diff : %f\t",diff_us);				//DEBUG
        printf("RPM : %u\n",rpm);

    }
    
}

/*-----------------------------------------------------------*/
static void diagnosticTask( void* pvParameters ){

    TickType_t xNextWakeTime;
    const TickType_t xBlockTime = mainDIAG_TICK_FREQUENCY;
    
    /* Prevent the compiler warning about the unused parameter. */
    ( void ) pvParameters;
    xNextWakeTime = xTaskGetTickCount();

    unsigned long int avg_slack=0;
	int i = 0;
	int rounds = 100;

    for( ; ;){
       
        vTaskDelayUntil( &xNextWakeTime, xBlockTime );

        xSemaphoreTake(slack_rt1.lock, portMAX_DELAY);
        xSemaphoreTake(slack_rt2.lock, portMAX_DELAY);

        avg_slack += (slack_rt1.slack_time + slack_rt2.slack_time)/2 * 1000; 	//average in microseconds

        xSemaphoreGive(slack_rt1.lock);
        xSemaphoreGive(slack_rt2.lock);
    
        i++;
		if(i == rounds){
			avg_slack = avg_slack/rounds;
			printf("**********SLACK TIME: %ld us**********\n",avg_slack);
			i = 0;
		}
    }

}