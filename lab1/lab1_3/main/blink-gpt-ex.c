//Isabella Phung
//blinks led on esp32c3 board
//example written by chat-gpt for cse103

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#define LED_GPIO 7 //led on esp32c3 board

void app_main(void)
{
    // Configure the IOMUX register for pad LED_GPIO (some boards need this)
    // puts pin into gpio mode bc pins have different functions
    esp_rom_gpio_pad_select_gpio(LED_GPIO);

    // Set the GPIO as a push/pull output
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    while(1) {
	// blink example uses a variable for the led state and inverts it
        // Turn the LED on
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait for 1000ms

        // Turn the LED off
        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait for 1000ms
    }
}

