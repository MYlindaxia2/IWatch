#include "../hardware/oled/OLED.h"
#include "../hardware/rtc/MyRTC.h"
#include "../utils/uiExchange.h"

enum {UPDATE_DATE, COUNT};
uint8_t curId = UPDATE_DATE;
void UpdateDatePage(void)
{
    MyRTC_ReadTime();
    OLED_ShowString(0,  0, "返回", OLED_8X16);
    OLED_Printf(0, 16, OLED_8X16, "年:%04d",  MyRTC_Time[0]);
    OLED_Printf(0, 32, OLED_8X16, "月:%02d", MyRTC_Time[1]);
    OLED_Printf(0, 48, OLED_8X16, "日:%02d",   MyRTC_Time[2]);
}

void UpdateTimePage(void)
{
    MyRTC_ReadTime();
    OLED_Printf(0, 0, OLED_8X16, "时:%02d",  MyRTC_Time[3]);
    OLED_Printf(0, 16, OLED_8X16, "分:%02d", MyRTC_Time[4]);
    OLED_Printf(0, 32, OLED_8X16, "秒:%02d",   MyRTC_Time[5]);
}


const Item dateItems[] =
{
    { { 0,  0, 32, 16 }, ACT_BACK, 0, 0, 0 },     /* EXIT  */
    { { 0, 16, 16, 16 }, ACT_EDIT, 0, 0, 0 },     /* Year  */
    { { 0, 32, 16, 16 }, ACT_EDIT, 0, 0, 1 },     /* Month */
    { { 0, 48, 16, 16 }, ACT_EDIT, 0, 0, 2 },     /* Day   */
};

const Item timeItems[] = 
{
    { { 0,  0, 16, 16 }, ACT_EDIT, 0, 0, 3 },     /* 时  */
    { { 0, 16, 16, 16 }, ACT_EDIT, 0, 0, 4 },     /* 分  */
    { { 0, 32, 16, 16 }, ACT_EDIT, 0, 0, 5 },     /* 秒 */
};