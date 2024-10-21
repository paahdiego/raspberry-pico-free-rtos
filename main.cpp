#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include <stdio.h>
#include <pico/stdio.h>
#include <pico/stdlib.h>
#include <pico/lwip_freertos.h>
#include <pico/async_context_freertos.h>

#include <lwip/raw.h>
#include <lwip/tcp.h>

const uint LED_PIN = 5;

struct SampleTaskArgT{
    TickType_t period;
    TickType_t delay;
    int pulsePeriod;
    int pulseCount;
    int priority;
    SemaphoreHandle_t handler;
};

void look_busy(int pinNumber, int pulses, int highs, int lows){
    for(int pulse = 0; pulse < pulses; pulse++){
        for(int high = 0; high < highs; high++){
            gpio_put(pinNumber, 1);
        }
        for(int low = 0; low < lows; low++){
            gpio_put(pinNumber, 0);       
        }
    }
}


static void sample_task(SampleTaskArgT * args) {
    TickType_t xLastWakeTime = 0;
    
    UBaseType_t priority = uxTaskPriorityGet(NULL);
    vTaskCoreAffinitySet(NULL, 1); 

    int high = int(args->pulsePeriod * priority / configMAX_PRIORITIES);
    int low = args->pulsePeriod - high;
    
    vTaskDelayUntil(&xLastWakeTime, args->delay);
    while(true) {
        look_busy(LED_PIN, args->pulseCount, high, low);
        xSemaphoreTake(args->handler, portMAX_DELAY);
        look_busy(LED_PIN, args->pulseCount, high, low);
        xSemaphoreGive(args->handler);
        look_busy(LED_PIN, args->pulseCount, high, low);
        vTaskDelayUntil(&xLastWakeTime, args->period);
    }
}

int main( void ) {
    
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    SemaphoreHandle_t binaryHandlerA = xSemaphoreCreateMutex();
    // xSemaphoreGive(binaryHandlerA);
    SemaphoreHandle_t binaryHandlerB = xSemaphoreCreateMutex();
    // xSemaphoreGive(binaryHandlerB);
    // SemaphoreHandle_t mutexHandler = xSemaphoreCreateMutex();

    // period, delay, pulsePeriod, pulseCount, priority, handler;
    SampleTaskArgT task1Args { 20, 0, 200, 200, 10, binaryHandlerA};
    SampleTaskArgT task2Args { 20, 2, 200, 200, 18, binaryHandlerB};
    SampleTaskArgT task3Args { 20, 4, 200, 200, 25, binaryHandlerA};
    
    
    stdio_init_all();

    printf("Creating tasks...\n");
    xTaskCreate((TaskFunction_t)sample_task, "sample", 2048, &task1Args, task1Args.priority, NULL);
    xTaskCreate((TaskFunction_t)sample_task, "sample", 2048, &task2Args, task2Args.priority, NULL);
    xTaskCreate((TaskFunction_t)sample_task, "sample", 2048, &task3Args, task3Args.priority, NULL);

    printf("Starting FreeRTOS-SMP scheduler...\n");
    vTaskStartScheduler();
    return 0;
}
/*-----------------------------------------------------------*/
