//written by Isabella Phung for CSE121 Spring 2023
//prints a message on a Gravity LCD1602 RGB Backlight Module
//using an esp32c3

#include <stdio.h>
#include "DFRobot_LCD.h"
#include "temp.c"
#include <stdbool.h>

DFRobot_LCD lcd(16,2);  //16 characters and 2 lines of show

extern "C" void app_main()
{

    lcd.init();
    lcd.display();
    lcd.clear();
    lcd.home();
    lcd.setRGB(255,0,0);
    char temp[9];
    char hum[8];
    float temperatureInC, tempInF, humidity;
    while (true) {
        if (shtc3_read_data(&temperatureInC, &humidity) == ESP_OK) { //reads data
            tempInF = convertToF(temperatureInC); //converts C to F
            lcd.setCursor(0,0);
            sprintf(temp, "Temp: %dC", (int)temperatureInC);
            lcd.printstr(temp);
            lcd.setCursor(0,1);
            sprintf(hum, "Hum: %d%%", (int)humidity);
            lcd.printstr(hum);
            ESP_LOGI(TAG, "Temperature is %dC (or %dF) with a %d%% humidity", (int)temperatureInC, (int)tempInF, (int)humidity);
        } else {
            ESP_LOGE(TAG, "Failed to read temperature and humidity");
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay 1 seconds
    }
}
