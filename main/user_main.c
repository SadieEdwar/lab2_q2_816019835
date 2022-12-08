/* gpio example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_system.h"

static const char *TAG = "main";
static const char *active_delay_tag = "Actively waited ...";
static const char *status_message = "Status message";

/**
 * Brief:
 * This test code shows how to configure gpio and how to use mutex
 * GPIO status messages
 * GPIO2: output
 */

#define GPIO_OUTPUT_IO_0     2
#define GPIO_OUTPUT_PIN_SEL  (1ULL<<GPIO_OUTPUT_IO_0) 

//Global variables
//static xQueueHandle gpio_evt_queue = NULL;
static SemaphoreHandle_t xSemaphore = NULL;

static TaskHandle_t Handle1 = NULL;

static TaskHandle_t Handle2 = NULL;

static TaskHandle_t Handle3 = NULL;

static void active_delay()
{
	//xTaskGetTickCount returns the count of ticks since vTaskStartScheduler was called, therefore it would be used to check when 500ms has passed since the current ticks of scheduler.
	uint32_t start_time = xTaskGetTickCount();
	uint32_t time_500 = xTaskGetTickCount();
	uint32_t end_time = start_time + ( 500 / portTICK_RATE_MS);
	//check for when the 500ms has passed
	while (time_500 < end_time)
	{
		//Continuously updates time since scheduler was turned on to check if 500ms has passed.
		time_500 = xTaskGetTickCount();
	}
}

static void gpio_task_1_sharingPin( void *arg )
{
    for (;;)
    {
    if ( xSemaphore != NULL )
    {
       /* See if we can obtain the semaphore.  If the semaphore is not available wait 10 ticks to see if it becomes free. */
       if( xSemaphoreTake( xSemaphore, ( TickType_t ) 10 ) == pdTRUE )
        {
       /* Accessing shared resources , GPIO pin 2 Turning on the LED connected to GPIO pin 2 */
		gpio_set_level(GPIO_OUTPUT_IO_0, 1);
                active_delay();
		ESP_LOGI(active_delay_tag, "500ms\n");

       /*Finished accessing shared resource. Release the semaphore. */
       	        xSemaphoreGive( xSemaphore );
   	        printf("Giving semaphore\n");
	        vTaskDelay(1000 / portTICK_RATE_MS);
    }
    else
    {
	    /*Semaphore was not obtain and therefore connot access the shared resource safely. */
	    printf("Waiting for the semaphore to release resource from low\n");
    }
}
}
}

static void gpio_task_2_sharingPin( void *arg )
{
    for (;;)
    {
    if ( xSemaphore != NULL )
    {
       /* See if we can obtain the semaphore.  If the semaphore is not available wait 10 ticks to see if it becomes free. */
       if( xSemaphoreTake( xSemaphore, ( TickType_t ) 10 ) == pdTRUE )
        {
       /* Accessing shared resources , GPIO pin 2 Turning on the LED connected to GPIO pin 2 */
                gpio_set_level(GPIO_OUTPUT_IO_0, 0);
                active_delay();
                ESP_LOGI(active_delay_tag, "500ms\n");

       /*Finished accessing shared resource. Release the semaphore. */
                xSemaphoreGive( xSemaphore );
                printf("Giving semaphore\n");
                vTaskDelay(1000 / portTICK_PERIOD_MS);
   }
    else
    {
	    /*Semaphore was not obtain and therefore connot access the shared resource safely. */
            printf("Waiting for the semaphore to release resource from high\n");
    }
}
}
}


static void gpio_task_3_message(void *arg)
{
    for (;;)
    {

            if (gpio_get_level(GPIO_OUTPUT_IO_0))
        	{
	          ESP_LOGI(status_message,"Led is currently on, GPIO2 is high\n");
                }
	    else
	        {
	          ESP_LOGI(status_message, "Led is currently off, GPIO2 is low\n");
	        }
	    vTaskDelay(1000 / portTICK_PERIOD_MS);
}
}

void app_main(void)
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to e.g GPIO 2
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config( &io_conf ) ;

  
    //Creating mutex
    xSemaphore = xSemaphoreCreateMutex();
    //start task1
    Handle1 = xTaskCreate(gpio_task_1_sharingPin, "gpio_task_1_sharingPin", 2048, NULL, 10, NULL);
    //start task2
    Handle2 = xTaskCreate(gpio_task_2_sharingPin, "gpio_task_2_sharingPin", 2048, NULL, 10, NULL);
    //start task3
    Handle3 = xTaskCreate(gpio_task_3_message, "gpio_task_3_message", 2048, NULL, 10, NULL);
   
    int cnt = 0;
    while (1)
    {
	    ESP_LOGI(TAG, "cnt: %d\n", cnt++);
	    printf("Checker\n");
	    vTaskDelay( 1000/ portTICK_PERIOD_MS);

    }
}
