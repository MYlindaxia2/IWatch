#include "../stm32f10x.h"
#include "../oled/OLED.h"
#include "../rtc/MyRTC.h"
#include "../key/Key.h"
uint8_t index = 0;
void ShowTime(void)
{
    MyRTC_ReadTime();
    OLED_Printf(0,0,OLED_6X8,"%04d-%02d-%02d", MyRTC_Time[0], MyRTC_Time[1], MyRTC_Time[2]);
    OLED_Printf(0,20,OLED_24X24, "%02d:%02d", MyRTC_Time[3], MyRTC_Time[4]);
    OLED_ShowString(0,50,"菜单", OLED_8X16);
    OLED_ShowString(96,50,"设置", OLED_8X16);
}
void ShowMasterPage(void)
{
    ShowTime();
    uint8_t key = KeyGetNumState();
    if(key == 1 && index <3)    index ++;
    if(index >= 3) index = 0;
    
    if(index == 1)
        OLED_ReverseArea(0, 50, 32, 16);
    if(index == 2)
        OLED_ReverseArea(96,50, 32, 16);
    OLED_Update();
}