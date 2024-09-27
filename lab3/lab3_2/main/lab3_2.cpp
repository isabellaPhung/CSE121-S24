//written by Isabella Phung for CSE121 Spring 2023
//prints a message on a Gravity LCD1602 RGB Backlight Module
//using an esp32c3

#include <stdio.h>
#include "DFRobot_LCD.h"
#include <stdbool.h>

DFRobot_LCD lcd(16,2);  //16 characters and 2 lines of show

extern "C" void app_main()
{

    lcd.init();
    lcd.display();
    lcd.clear();
    lcd.home();
    lcd.setRGB(255,0,0);
    lcd.printstr("Hello CSE121!");
    lcd.setCursor(0,1);
    lcd.printstr("Isabella");
}
