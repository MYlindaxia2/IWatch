#include "../stm32f10x.h"
#include "../oled/OLED.h"
#include "../rtc/MyRTC.h"
#include "../key/Key.h"


// #define ITEM_COUNT(a)  ((uint8_t)sizeof((a)/sizeof((a)[0])))
#define PG_NONE 0xFF
#define ITEM_COUNT(a) (uint8_t) (sizeof(a) / sizeof((a)[0]))
#define LINK_NONE {PG_NONE, 0}
typedef struct {
    uint8_t page;
    uint8_t sel;
}Link;
typedef struct {
    uint16_t x,y,w,h;
}HotSpot;

typedef enum {
    ACT_NONE = 0,
    ACT_GOTO = 1,
    ACT_BACK = 2,
}ActKind;

typedef struct {
    HotSpot hot;
    ActKind act;
    uint8_t target;
    void (*call)(uint8_t);
    uint8_t arg;
}Item;

typedef struct{
    const Item *items;
    uint8_t count;
    uint8_t sel;
    void (*draw)(void);
    Link leftTo;
    Link rightTo;
}Page;

enum {MAIN, MENU, EDIT, UP_DATE,UP_TIME, COUNT};



static void ShowTime(void)
{
    MyRTC_ReadTime();
    OLED_Printf(0, 0, OLED_6X8, "%04d-%02d-%02d",
                MyRTC_Time[0], MyRTC_Time[1], MyRTC_Time[2]);
    OLED_Printf(0, 20, OLED_24X24, "%02d:%02d", MyRTC_Time[3], MyRTC_Time[4]);
    OLED_ShowString(0,  50, "菜单", OLED_8X16);
    OLED_ShowString(96, 50, "设置", OLED_8X16);
}

static void EditPage(void)
{
    OLED_ShowString(0,  0, "返回",OLED_8X16);
    OLED_ShowString(0, 16, "设置日期",OLED_8X16);
}

static void UpdateDatePage(void)
{
    MyRTC_ReadTime();
    OLED_ShowString(0,  0, "返回", OLED_8X16);
    OLED_Printf(0, 16, OLED_8X16, "年:%04d",  MyRTC_Time[0]);
    OLED_Printf(0, 32, OLED_8X16, "月:%02d", MyRTC_Time[1]);
    OLED_Printf(0, 48, OLED_8X16, "日:%02d",   MyRTC_Time[2]);
}

static void UpdateTimePage(void)
{
    MyRTC_ReadTime();
    OLED_Printf(0, 0, OLED_8X16, "时:%02d",  MyRTC_Time[3]);
    OLED_Printf(0, 16, OLED_8X16, "分:%02d", MyRTC_Time[4]);
    OLED_Printf(0, 32, OLED_8X16, "秒:%02d",   MyRTC_Time[5]);
}

static const Item mainItems[] = 
{
    {{0,50,32,16}, ACT_GOTO, MENU, 0, 0},
    {{96,50,32,16}, ACT_GOTO, EDIT, 0, 0},
};

static const Item editItems[] =
{
    { { 0,  0, 32, 16 }, ACT_BACK, 0,0, 0 },   /* EXIT        */
    { { 0, 16, 64, 16 }, ACT_GOTO, UP_DATE, 0, 0 },   /* Update Date */
};

static const Item dateItems[] =
{
    { { 0,  0, 32, 16 }, ACT_BACK, 0, 0, 0 },     /* EXIT  */
    { { 0, 16, 16, 16 }, ACT_NONE, 0, 0, 0 },     /* Year  */
    { { 0, 32, 16, 16 }, ACT_NONE, 0, 0, 1 },     /* Month */
    { { 0, 48, 16, 16 }, ACT_NONE, 0, 0, 2 },     /* Day   */
};

static const Item timeItems[] = 
{
    { { 0,  0, 16, 16 }, ACT_NONE, 0, 0, 3 },     /* 时  */
    { { 0, 16, 16, 16 }, ACT_NONE, 0, 0, 4 },     /* 分  */
    { { 0, 32, 16, 16 }, ACT_NONE, 0, 0, 5 },     /* 秒 */
};

static Page pages[COUNT] = 
{
    [MAIN] = {mainItems, ITEM_COUNT(mainItems),0,ShowTime, LINK_NONE,LINK_NONE},
    [EDIT] = {editItems, ITEM_COUNT(editItems),0,EditPage,LINK_NONE,LINK_NONE},
    [UP_DATE] = {dateItems, ITEM_COUNT(dateItems),0,UpdateDatePage,{UP_TIME, 0},LINK_NONE},
    [UP_TIME] = {timeItems, ITEM_COUNT(timeItems),0,UpdateTimePage, {UP_DATE, 0},{UP_DATE, 3}},
};

#define NAV_MAX 6
static uint8_t navStack[NAV_MAX];
static uint8_t navDepth = 0;
static uint8_t curId = MAIN;

static void Nav_Switch(uint8_t id, uint8_t sel)
{
    if(sel >= pages[id].count) sel = 0;
    curId = id;
    pages[id].sel = sel;
}
static void Nav_GotoAt(uint8_t id,uint8_t sel)
{
    if(sel >= pages[id].count) sel = 0;
    if(navDepth < NAV_MAX) navStack[navDepth++] = curId;
    curId = id;
    pages[id].sel = sel;
}

static void Nav_Goto(uint8_t id)
{
    Nav_GotoAt(id, 0);
}

static void Nav_Back(void)
{
    if(navDepth > 0)
    {
        curId = navStack[--navDepth];
    }
}

static void Ui_Input(void)
{
    KeyFun k = KeyGetNumState();
    Page *p = &pages[curId];

    if(k == NONE || p->count == 0) return;
    if(k == LEFT)
    {
        if(p->sel + 1 >= p->count && p->leftTo.page != PG_NONE)
        {
            Nav_Switch(p->leftTo.page, p->leftTo.sel);
            return;
        }
        p->sel = (uint8_t)((p->sel + 1) % p->count);
        return;
    }
    if(k == RIGHT)
    {
        if(p->sel == 0 && p->rightTo.page != PG_NONE)
        {
            Nav_Switch(p->rightTo.page, p->rightTo.sel);
            return;
        }
        p->sel = (uint8_t)((p->sel + p->count - 1) % p->count);
        return;
    }

    const Item *it = &p->items[p->sel];
    if(it->call) it->call(it->arg);

    if(it->act == ACT_GOTO) Nav_Goto(it->target);
    else if(it->act == ACT_BACK) Nav_Back();
}

static void Ui_Draw(void)
{
    Page *p = &pages[curId];
    p->draw();

    if (p->count == 0) return;

    const HotSpot *h = &p->items[p->sel].hot;
    OLED_ReverseArea(h->x, h->y, h->w, h->h);
}

void ShowMasterPage(void)
{
    OLED_Clear();
    Ui_Input();     /* 先处理输入，可能已经换页 */
    Ui_Draw();      /* 再按最新状态画，所以进页面 */
    OLED_Update();
}
