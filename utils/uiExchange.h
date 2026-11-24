#include <stdint.h>
#ifndef __UIEXCHANGE_H
#define __UIEXCHANGE_H
#define NAV_MAX 6
#define PG_NONE 0xFF
#define ITEM_COUNT(a) (uint8_t) (sizeof(a) / sizeof((a)[0]))
#define LINK_NONE {PG_NONE, 0}

extern uint8_t navStack[NAV_MAX];
extern uint8_t navDepth;
// 应该是MAIN
extern uint8_t curId;

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
    ACT_EDIT = 3,
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


void Nav_Switch(uint8_t id, uint8_t sel, Page *pages);

void Nav_GotoAt(uint8_t id,uint8_t sel, Page *pages);

void Nav_Goto(uint8_t id);

void Nav_Back(void);

void Ui_Input(Page *pages);

void Ui_Draw(Page *pages);

#endif // !__UIEXCHANGE_H

