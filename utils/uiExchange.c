#include "uiExchange.h"
#include "../hardware/key/Key.h"
#include "../hardware/oled/OLED.h"
#include "../pages/updateDate/updateTime.h"
/* ============================================================================
 * 这个文件是整个 UI 的"内核"，只干三件事：
 *   1. 记住"现在在哪一页"（curId）和"从哪来的"（navStack）
 *   2. 把按键翻译成状态变化（Ui_Input）
 *   3. 把状态画到屏幕上（Ui_Draw）
 *
 * 它完全不知道有几个页面、每个页面长什么样 —— 那些都是 pages[] 里的数据。
 * 这就是为什么新增页面不用改这个文件。
 * ========================================================================== */

/* ----------------------------------------------------------------------------
 * 内核状态
 *
 * 为什么用 static：curId 只能由本文件的 Nav_* 函数修改。
 * 一旦它变成全局外部可写，就会出现"页面在绘制时偷偷改当前页"的情况
 * （之前 UpdateDatePage 里调 ChangeCurID 就是这么把导航搞坏的）。
 * -------------------------------------------------------------------------- */
static uint8_t navStack[NAV_MAX];   /* 返回栈：存的是页号，不是指针 */
static uint8_t navDepth = 0;        /* 栈深度，同时也是"下一格写哪里" */
static uint8_t curId   = PG_MAIN;   /* 开机在主页 */
static uint8_t editState = 0;       /* 0关闭修改、1开启修改 */
/* 取当前页。
 * 注意返回的是指针：pages[] 是指针数组，pages[curId] 才是 Page 本体。
 * 早期写成 &pages[curId] 是个经典错误 —— pages 是 Page* 的数组，
 * &pages[1] 是"第二个指针的地址"，被当成 Page* 用就会按 Page 的大小
 * 往结构体外面走，最后 call 到乱七八糟的地址上（HardFault）。 */
static Page *Ui_Cur(Page *pages[])
{
    if (curId >= PG_COUNT) return 0;
    return pages[curId];
}

/* ============================================================================
 * 导航：三个函数，区别只在"压不压栈"
 *
 *   Nav_Switch  不压栈 —— 同一层的左右平移（翻页）
 *   Nav_GotoAt  压栈   —— 进入子页，记住来处，可指定光标落点
 *   Nav_Back    出栈   —— 回到来处
 *
 * 为什么翻页不能压栈：日期页和时间页左右翻几次，栈里就会堆满
 * PG_DATE/PG_TIME，之后再按"返回"，弹出的就是时间页而不是设置页。
 * ========================================================================== */

/* 平级平移（翻页） */
void Nav_Switch(uint8_t id, uint8_t sel, Page *pages[])
{
    /* 目标页可能还没写（pages[id] == 0），直接放弃，绝不继续往下走。
     * 这就是主页按"菜单"不会崩的原因。 */
    if (id >= PG_COUNT || pages[id] == 0) return;

    /* sel 是别人给的，可能越界（比如页面选项数改少了），这里兜一下底 */
    if (sel >= pages[id]->count) sel = 0;

    curId = id;
    pages[id]->sel = sel;
}

/* 进入子页 */
void Nav_GotoAt(uint8_t id, uint8_t sel, Page *pages[])
{
    if (id >= PG_COUNT || pages[id] == 0) return;
    if (sel >= pages[id]->count) sel = 0;

    /* 只有"进子页"才记来处；栈满就丢弃，宁可返回不回去也不能越界写内存 */
    if (navDepth < NAV_MAX) navStack[navDepth++] = curId;

    curId = id;
    pages[id]->sel = sel;
}

/* 返回上一层 */
void Nav_Back(void)
{
    /* 空栈就不动，避免 navDepth 下溢成 255 */
    if (navDepth > 0) curId = navStack[--navDepth];
}



/* ============================================================================
 * 输入：所有页面共用这一份
 *
 * 这里从头到尾没有一句"如果当前是主页则……"——
 * 因为"现在在哪一页"已经蕴含在 curId 里了，而 curId 又只指向 p。
 * 页面之间的差异全在数据（items/leftTo/rightTo）里，不在这个函数里。
 * ========================================================================== */
void Ui_Input(Page *pages[])
{
    KeyFun k = KeyGetNumState();
    Page  *p = Ui_Cur(pages);       /* p 就是"我这一页"，不用再判断是第几层 */
    
    if (p == 0) return;             /* 页面还没写 */
    if (k == NONE || p->count == 0) return;   /* 没按键 / 页面没有选项 */
    const Item *it = &p->items[p->sel];

    if(editState)
    {
        switch (k)
        {
        case LEFT:
            AddDateOrTime(it->arg);
            return;
        case RIGHT:
            SubDateOrTime(it->arg);
            return;
        case ENTER:
            editState = 0;
            return;
        default:
            break;
        }
    }
    /* ---- 左键：往下走一格（sel + 1） ---- */
    if (k == LEFT)
    {
        /* 已经在最后一项了，还往下走，就翻到"下一页"。
         * leftTo.page == PG_NONE 表示这页没有下一页，那就环绕回第一项。 */
        if (p->sel + 1 >= p->count && p->leftTo.page != PG_NONE)
        {
            Nav_Switch(p->leftTo.page, p->leftTo.sel, pages);
            return;
        }

        /* % count 同时实现了"到最后一项回到 0"，不需要 if(>n) sel=0 */
        p->sel = (uint8_t)((p->sel + 1) % p->count);
        return;
    }

    /* ---- 右键：往上走一格（sel - 1） ---- */
    if (k == RIGHT)
    {
        if (p->sel == 0 && p->rightTo.page != PG_NONE)
        {
            Nav_Switch(p->rightTo.page, p->rightTo.sel, pages);
            return;
        }

        /* 这里借用了无符号数的回绕来做 -1：
         * sel=0 时 (0 + count - 1) % count == count-1，正好到末项。
         * 比 "if(sel==0) sel=count-1; else sel--;" 少一个分支，也不会出错。 */
        p->sel = (uint8_t)((p->sel + p->count - 1) % p->count);
        return;
    }

    /* ---- 确认键 ---- */
    {

        /* 先执行附加动作，再处理换页：
         * 这样"改完日期立刻跳走"这种组合只需要在数据里写一个 call + 一个 act。 */
        if (it->call) it->call(it->arg);

        switch (it->act)
        {
        case ACT_GOTO: Nav_GotoAt(it->target, 0, pages); break;   /* 落在目标页第 0 项 */
        case ACT_BACK: Nav_Back();                       break;
        case ACT_EDIT: editState = 1;                    break;
        default:                                         break;   /* ACT_NONE / ACT_EDIT 暂无动作 */
        }
        return;
    }
}

/* ============================================================================
 * 绘制：分成两步，顺序不能反
 *
 *   1. p->draw()   画这一页的静态内容（文字、时间）
 *   2. 画高亮      只有一块，位置由 items[sel].hot 决定
 *
 * 高亮统一在这里画，所以每个页面的 draw 函数里不用再写一堆
 * "if (sel == 1) 反显这里 else if (sel == 2) 反显那里"。
 * ========================================================================== */
void Ui_Draw(Page *pages[])
{
    Page *p = Ui_Cur(pages);

    if (p == 0) return;

    /* draw 判空：页面表里可能有占位项（draw 为 NULL），裸调会跳到地址 0 */
    if (p->draw) p->draw();

    if (p->count == 0) return;      /* 没选项就没高亮 */

    {
        const HotSpot *h = &p->items[p->sel].hot;
        OLED_ReverseArea(h->x, h->y, h->w, h->h);
    }
}

/* ============================================================================
 * 主循环每帧调一次（替代原来的 ShowMasterPage）
 *
 * 顺序是刻意的：先输入、后绘制。
 * 这样按 ENTER 换页的当帧就能画出新页面，不会闪一帧旧内容。
 *
 * 注意 OLED_ReverseArea 是"异或翻转"，不是"设为反显"，
 * 所以必须每帧先 Clear 再画，否则同一块区域会被反复取反、看着像在闪。
 * 以后要做"局部刷新"优化时，这一点是前提。
 * ========================================================================== */
void Ui_Task(void)
{
    OLED_Clear();
    Ui_Input(pages);
    Ui_Draw(pages);
    OLED_Update();
}
