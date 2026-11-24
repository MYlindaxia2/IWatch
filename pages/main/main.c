#include "../stm32f10x.h"
#include "../hardware/oled/OLED.h"
#include "../hardware/rtc/MyRTC.h"
#include "../utils/uiExchange.h"
#include "../setting/setting.h"
#include "../updateDate/updateTime.h"
#include "../setting/setting.h"

// enum {MAIN, MENU, EDIT, UP_DATE,UP_TIME, COUNT};
enum {MAIN, MENU, EDIT, COUNT};
uint8_t curId = MAIN;

static void ShowTime(void)
{
    MyRTC_ReadTime();
    OLED_Printf(0, 0, OLED_6X8, "%04d-%02d-%02d",
                MyRTC_Time[0], MyRTC_Time[1], MyRTC_Time[2]);
    OLED_Printf(0, 20, OLED_24X24, "%02d:%02d", MyRTC_Time[3], MyRTC_Time[4]);
    OLED_ShowString(0,  50, "菜单", OLED_8X16);
    OLED_ShowString(96, 50, "设置", OLED_8X16);
}

static const Item mainItems[] = 
{
    {{0,50,32,16}, ACT_GOTO, MENU, 0, 0},
    {{96,50,32,16}, ACT_GOTO, EDIT, 0, 0},
};

Page mainPage = {mainItems, ITEM_COUNT(mainItems), 0, ShowTime, LINK_NONE,LINK_NONE};

Page *pages[COUNT] = 
{
    [MAIN] = &mainPage,
    [EDIT] = &editPage,
    // [UP_DATE] = {dateItems, ITEM_COUNT(dateItems),0,UpdateDatePage,{UP_TIME, 0},LINK_NONE},
    // [UP_TIME] = {timeItems, ITEM_COUNT(timeItems),0,UpdateTimePage, {UP_DATE, 0},{UP_DATE, 3}},
};

void ShowMasterPage(void)
{
    OLED_Clear();
    Ui_Input(*pages);     /* 先处理输入，可能已经换页 */
    Ui_Draw(*pages);      /* 再按最新状态画，所以进页面 */
    OLED_Update();
}
