#include "../hardware/oled/OLED.h"
#include "../utils/uiExchange.h"
enum {EDIT,UPDATE_DATE, COUNT};
uint8_t curId = EDIT;

const Item editItems[] =
{
    { { 0,  0, 32, 16 }, ACT_BACK, 0,0, 0 },   /* EXIT        */
    // { { 0, 16, 64, 16 }, ACT_GOTO, UP_DATE, 0, 0 },   /* Update Date */
    { { 0, 16, 64, 16 }, ACT_GOTO, 0, 0, 0 },   /* Update Date */
};

void EditPage(void)
{
    OLED_ShowString(0,  0, "返回",OLED_8X16);
    OLED_ShowString(0, 16, "设置日期",OLED_8X16);
}

Page editPage = {editItems, ITEM_COUNT(editItems), 0, EditPage, LINK_NONE,LINK_NONE};
