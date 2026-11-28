#include "../stm32f10x.h"
#include "../../utils/uiExchange.h"
#include "../../hardware/oled/OLED.h"
#include "../../hardware/led/LED.h"

static void LightPage(void)
{
    OLED_ShowString(0,0,"返回", OLED_8X16);
    OLED_ShowString(24, 24, "OFF", OLED_8X16);
    OLED_ShowString(80, 24, "ON", OLED_8X16);
}

static const Item lightItems[] =
{
    {{0,0,32,16},ACT_BACK, 0,0,0},
    {{24,24,24,16}, ACT_GOTO_FUNCTION, 0, 0, 0,LED_OFF},
    {{80,24, 16, 16}, ACT_GOTO_FUNCTION, 0, 0, 0, LED_ON}
};


Page lightPage = {lightItems, ITEM_COUNT(lightItems), 0, LightPage, LINK_NONE, LINK_NONE};
