/*
 * ************************************************
 * 
 *              STM32 blink gcc demo
 * 
 *  CPU: STM32F103C8
 *  PIN: PA1
 * 
 * ************************************************
*/

#include "stm32f10x.h"
#include "../hardware/oled/OLED.h"
#include "../hardware/rtc/MyRTC.h"
#include "../hardware/delay/Delay.h"
#include "../hardware/key/Key.h"
#include "../hardware/led/LED.h"
#include "../hardware/tim/Timer.h"
#include "../pages/main/main.h"
int main()
{
    OLED_Init();
    MyRTC_Init();
    LED_Init();
    Key_Init();
    Timer_Init();
    while (1)
    {
        ShowMasterPage();
    }
}
