//Generated by ChatGPT, edited by Isabella Phung to include temperature reading capabilities
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "temp.c"

#define TRIGGER_PIN GPIO_NUM_0
#define ECHO_PIN GPIO_NUM_1

//static const char *TAG = "SR04";

void sr04_init(void)
{
    esp_rom_gpio_pad_select_gpio(TRIGGER_PIN); //selects GPIO pin 0, trigger
    gpio_set_direction(TRIGGER_PIN, GPIO_MODE_OUTPUT); //sets as output
    esp_rom_gpio_pad_select_gpio(ECHO_PIN); //selects pin 1, echo
    gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT); //sets as input
}

int64_t measure_distance()
{
    // Send a 10us pulse to trigger the sensor
    gpio_set_level(TRIGGER_PIN, 0); //set to 0
    vTaskDelay(pdMS_TO_TICKS(2));  // Wait for 2 ms
    gpio_set_level(TRIGGER_PIN, 1); //set to 1
    esp_rom_delay_us(10);  // Wait for 10 us
    gpio_set_level(TRIGGER_PIN, 0); //set to 0

    // Wait for the echo pin to go high and measure the time
    int64_t start_time = 0;
    int64_t end_time = 0;
    while (gpio_get_level(ECHO_PIN) == 0){ //continuously gets time as pin is 0
        start_time = esp_timer_get_time();
    }
    while (gpio_get_level(ECHO_PIN) == 1){ //continuously gets time as pin is 1
        end_time = esp_timer_get_time();
    }

    // Calculate the distance in cm
    int64_t duration = end_time - start_time;

    //measure current temperature using onboard thermometer 
    float temperatureInC;
    float humidity;
    if (shtc3_read_data(&temperatureInC, &humidity) != ESP_OK) { //reads data and checks for error
        ESP_LOGE("Temp", "Failed to read temperature");
    }

    float distance = (duration / 2.0) * ((331+(0.61*temperatureInC))/10000.0); 
    //divided by 2 bc it measures roundtrip time, we want distance 1 way
    //duration is multiplied by speed of sound, 0.0343cm/s
    //need to implement affect of temp on speed of sound: v=331+(0.61*T)

    ESP_LOGI("SR04", "Measured distance: %.2f cm at %dC", distance, (int)temperatureInC);
    return distance;
}

void app_main(void)
{
    i2c_master_init();
    sr04_init();
    while (1)
    {
        measure_distance();
        vTaskDelay(pdMS_TO_TICKS(1000));  // Delay for 1 second
    }
}

