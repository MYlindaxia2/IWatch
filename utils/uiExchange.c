#include "uiExchange.h"
#include <stdint.h>
#include "../hardware/key/Key.h"
#include "../hardware/oled/OLED.h"

uint8_t navStack[NAV_MAX];
uint8_t navDepth = 0;

void Nav_Switch(uint8_t id, uint8_t sel, Page *pages)
{
    if(sel >= pages[id].count) sel = 0;
    curId = id;
    pages[id].sel = sel;
}
void Nav_GotoAt(uint8_t id,uint8_t sel, Page *pages)
{
    if(sel >= pages[id].count) sel = 0;
    if(navDepth < NAV_MAX) navStack[navDepth++] = curId;
    curId = id;
    pages[id].sel = sel;
}

void Nav_Back(void)
{
    if(navDepth > 0)
    {
        curId = navStack[--navDepth];
    }
}

void Ui_Input(Page *pages)
{
    KeyFun k = KeyGetNumState();
    Page *p = &pages[curId];

    if(k == ACT_EDIT)
    {
        
    }
    if(k == NONE || p->count == 0) return;
    if(k == LEFT)
    {
        if(p->sel + 1 >= p->count && p->leftTo.page != PG_NONE)
        {
            Nav_Switch(p->leftTo.page, p->leftTo.sel, pages);
            return;
        }
        p->sel = (uint8_t)((p->sel + 1) % p->count);
        return;
    }
    if(k == RIGHT)
    {
        if(p->sel == 0 && p->rightTo.page != PG_NONE)
        {
            Nav_Switch(p->rightTo.page, p->rightTo.sel, pages);
            return;
        }
        p->sel = (uint8_t)((p->sel + p->count - 1) % p->count);
        return;
    }

    const Item *it = &p->items[p->sel];
    if(it->call) it->call(it->arg);

    if(it->act == ACT_GOTO) Nav_GotoAt(it->target, 0, pages);
    else if(it->act == ACT_BACK) Nav_Back();
}

void Ui_Draw(Page *pages)
{
    Page *p = &pages[curId];
    p->draw();

    if (p->count == 0) return;

    const HotSpot *h = &p->items[p->sel].hot;
    OLED_ReverseArea(h->x, h->y, h->w, h->h);
}