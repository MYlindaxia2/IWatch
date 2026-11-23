#include "../stm32f10x.h"
#include "../oled/OLED.h"
#include "../rtc/MyRTC.h"
#include "../key/Key.h"

/* ==========================================================
 * 1. 类型定义
 * ========================================================== */

/* 屏幕上一块矩形区域，用来画高亮 */
typedef struct
{
    int16_t x, y;
    uint8_t w, h;
} Hotspot;

/* 按下某一项之后做什么 */
typedef enum
{
    ACT_NONE = 0,   /* 什么也不做（或只执行 call） */
    ACT_GOTO,       /* 进入另一页，压栈 */
    ACT_BACK        /* 返回上一层，出栈 */
} ActKind;

/* 一个可选项 */
typedef struct
{
    Hotspot hot;            /* 高亮区域 */
    ActKind act;            /* 动作类型 */
    uint8_t target;         /* ACT_GOTO 时的目标页号 */
    // 函数指针
    void  (*call)(uint8_t); /* 附加动作，可为 NULL，参数由 arg 传入 */
    uint8_t arg;            /* 传给 call 的参数 */
} Item;

/* 一页 */
typedef struct
{
    const Item *items;
    uint8_t     count;      /* items 个数，sel 恒在 0..count-1 */
    uint8_t     sel;        /* 当前选中项，页面自己记住 */
    void      (*draw)(void);/* 只画静态内容，高亮由 Ui_Draw 统一画 */
} Page;

/* 页号：以后加页面就在这里加一个 */
enum { PG_MAIN, PG_MENU, PG_EDIT, PG_DATE, PG_COUNT };

#define ITEM_COUNT(a)   ((uint8_t)(sizeof(a) / sizeof((a)[0])))

/* ==========================================================
 * 2. 各页面的绘制函数（只管画静态内容，不画高亮）
 * ========================================================== */

static void ShowTime(void)
{
    MyRTC_ReadTime();
    OLED_Printf(0, 0, OLED_6X8, "%04d-%02d-%02d",
                MyRTC_Time[0], MyRTC_Time[1], MyRTC_Time[2]);
    OLED_Printf(0, 20, OLED_24X24, "%02d:%02d", MyRTC_Time[3], MyRTC_Time[4]);
    OLED_ShowString(0,  50, "菜单", OLED_8X16);
    OLED_ShowString(96, 50, "设置", OLED_8X16);
}

static void MenuPage(void)
{
    OLED_ShowString(0,  0, "Stopwatch", OLED_8X16);
    OLED_ShowString(0, 16, "Alarm",     OLED_8X16);
    OLED_ShowString(0, 32, "About",     OLED_8X16);
    OLED_ShowString(0, 48, "Back",      OLED_8X16);
}

static void EditPage(void)
{
    OLED_ShowString(0,  0, "EXIT",        OLED_8X16);
    OLED_ShowString(0, 16, "Update Date", OLED_8X16);
}

static void UpdateDatePage(void)
{
    MyRTC_ReadTime();
    OLED_ShowString(0,  0, "EXIT", OLED_8X16);
    OLED_Printf(0, 16, OLED_8X16, "Year:%04d",  MyRTC_Time[0]);
    OLED_Printf(0, 32, OLED_8X16, "Month:%02d", MyRTC_Time[1]);
    OLED_Printf(0, 48, OLED_8X16, "Day:%02d",   MyRTC_Time[2]);
}

/* ==========================================================
 * 3. 动作函数（按 ENTER 时被调用，参数就是 Item.arg）
 * ========================================================== */

/* 把 MyRTC_Time 的某一项加 1，并写回 RTC 硬件
   field: 0=年 1=月 2=日 */
static void Date_Add(uint8_t field)
{
    MyRTC_ReadTime();           /* 先读硬件，避免在旧值上累加 */
    MyRTC_Time[field]++;

    if (field == 1 && MyRTC_Time[1] > 12) MyRTC_Time[1] = 1;
    if (field == 2 && MyRTC_Time[2] > 31) MyRTC_Time[2] = 1;

    MyRTC_SetTime();            /* 写回 RTC */
}

/* ==========================================================
 * 4. 各页面的选项表  —— 整个 UI 就是这几张表
 * ========================================================== */

/* 主页：菜单 / 设置 */
static const Item mainItems[] =
{
    { {  0, 50, 32, 16 }, ACT_GOTO, PG_MENU, NULL, 0 },   /* 菜单 */
    { { 96, 50, 32, 16 }, ACT_GOTO, PG_EDIT, NULL, 0 },   /* 设置 */
};

/* 菜单页：占位内容，功能做好了再改 act / call */
static const Item menuItems[] =
{
    { { 0,  0, 128, 16 }, ACT_NONE, 0, NULL, 0 },         /* Stopwatch */
    { { 0, 16, 128, 16 }, ACT_NONE, 0, NULL, 0 },         /* Alarm     */
    { { 0, 32, 128, 16 }, ACT_NONE, 0, NULL, 0 },         /* About     */
    { { 0, 48, 128, 16 }, ACT_BACK, 0, NULL, 0 },         /* Back      */
};

/* 设置页：EXIT / Update Date */
static const Item editItems[] =
{
    { { 0,  0, 128, 16 }, ACT_BACK, 0,       NULL, 0 },   /* EXIT        */
    { { 0, 16, 128, 16 }, ACT_GOTO, PG_DATE, NULL, 0 },   /* Update Date */
};

/* 日期页：EXIT / 年 / 月 / 日 */
static const Item dateItems[] =
{
    { { 0,  0, 128, 16 }, ACT_BACK, 0, NULL,     0 },     /* EXIT  */
    { { 0, 16, 128, 16 }, ACT_NONE, 0, Date_Add, 0 },     /* Year  */
    { { 0, 32, 128, 16 }, ACT_NONE, 0, Date_Add, 1 },     /* Month */
    { { 0, 48, 128, 16 }, ACT_NONE, 0, Date_Add, 2 },     /* Day   */
};

/* ==========================================================
 * 5. 页面表：页号 -> 选项表 + 绘制函数
 * ========================================================== */

static Page pages[PG_COUNT] =
{
    [PG_MAIN] = { mainItems, ITEM_COUNT(mainItems), 0, ShowTime       },
    [PG_MENU] = { menuItems, ITEM_COUNT(menuItems), 0, MenuPage       },
    [PG_EDIT] = { editItems, ITEM_COUNT(editItems), 0, EditPage       },
    [PG_DATE] = { dateItems, ITEM_COUNT(dateItems), 0, UpdateDatePage },
};

/* ==========================================================
 * 6. 导航：当前页 + 返回栈
 * ========================================================== */

#define NAV_MAX     4

static uint8_t navStack[NAV_MAX];   /* 存的是页号，整个栈只占 4 字节 */
static uint8_t navDepth = 0;
static uint8_t curId    = PG_MAIN;

static void Nav_Goto(uint8_t id)
{
    if (navDepth < NAV_MAX) navStack[navDepth++] = curId;  /* 记住来处 */
    curId = id;
    pages[curId].sel = 0;                                   /* 进新页面从第一项开始 */
}

static void Nav_Back(void)
{
    if (navDepth > 0) curId = navStack[--navDepth];
}

/* ==========================================================
 * 7. 输入：所有页面共用一份，完全不判断"我在第几层"
 * ========================================================== */

static void Ui_Input(void)
{
    KeyFun  k = KeyGetNumState();
    Page   *p = &pages[curId];

    if (k == NONE || p->count == 0) return;

    /* 左右只管搬动本页的 sel，跟别的页面无关 */
    if (k == LEFT)
    {
        p->sel = (uint8_t)((p->sel + 1) % p->count);
        return;
    }
    if (k == RIGHT)
    {
        p->sel = (uint8_t)((p->sel + p->count - 1) % p->count);
        return;
    }

    /* ENTER：先执行附加动作，再处理跳转 */
    const Item *it = &p->items[p->sel];
    if (it->call) it->call(it->arg);

    if      (it->act == ACT_GOTO) Nav_Goto(it->target);
    else if (it->act == ACT_BACK) Nav_Back();
}

/* ==========================================================
 * 8. 绘制：先画页面内容，再统一画高亮
 * ========================================================== */

static void Ui_Draw(void)
{
    Page *p = &pages[curId];
    p->draw();

    if (p->count == 0) return;

    const Hotspot *h = &p->items[p->sel].hot;
    OLED_ReverseArea(h->x, h->y, h->w, h->h);
}

/* ==========================================================
 * 9. 对外入口（main.c 不用改）
 * ========================================================== */

void ShowMasterPage(void)
{
    OLED_Clear();
    Ui_Input();     /* 先处理输入，可能已经换页 */
    Ui_Draw();      /* 再按最新状态画，所以进页面当帧就显示新内容 */
    OLED_Update();
}
