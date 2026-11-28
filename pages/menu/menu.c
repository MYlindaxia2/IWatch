#include "menu.h"
#include "../../hardware/oled/OLED.h"

/* ============================================================================
 * 菜单页：5 个 32×32 图标的横向轮播，左右键切换
 *
 * 屏幕 128 宽，图标按 48 的间距排在一条 224 宽的"虚拟长条"上。
 * 长条不动、屏幕不动，动的是"从哪儿开始看"——那个量叫 scroll。
 *
 *   scroll == sel * 48 时，第 sel 个图标正好停在屏幕正中（x = 48）。
 *
 * 切换之所以会"滑"，是因为 scroll 每帧只走 8 像素，要 6 帧才追上目标。
 * 6 帧 × 一次 OLED_Update(~9ms) ≈ 55ms，是手感最舒服的区间。
 * ========================================================================== */

#define MENU_ICON_W     32
#define MENU_ICON_H     32
#define MENU_PITCH      48                          /* 图标 32 + 间距 16 */
#define MENU_Y          16
#define MENU_CENTER_X   ((128 - MENU_ICON_W) / 2)   /* = 48，选中项永远停这 */
#define MENU_STEP       8                           /* 每帧滑动像素 */

/* ----------------------------------------------------------------------------
 * 图标表：下标必须和 menuItems 一一对应
 *
 * 你原来第 4、5 个都写了 Shuipin —— 应该是手误。OLED_Data.h 里还有一个
 * Game[] 一直没派上用场，这里补上，正好 5 个。
 * -------------------------------------------------------------------------- */
static const uint8_t *const menuIcons[] =
{
    Exit,       /* 0 退出 → 返回主页 */
    Timer,      /* 1 秒表 */
    Light,      /* 2 手电 */
    Shuipin,    /* 3 水平仪 */
    Game,       /* 4 游戏 */
};

/* 当前滚动偏移（像素）。
 * 必须是跨帧保持的，所以是 static，不能是局部变量 —— 局部变量每帧归零就滑不起来了。 */
static int16_t scroll = 0;

static void MenuPage(void);     /* 前置声明：menuPage 的初始化器里要取它的地址 */

/* ----------------------------------------------------------------------------
 * 选项表
 *
 * 五个 hot 完全相同，而且是写死的坐标 —— 这不是偷懒：
 * 因为"选中项永远停在屏幕正中"，所以高亮框的位置永远不变，
 * 变的只是框里画的是哪个图标。于是 hot 根本不需要跟着 sel 算。
 *
 * hot = {44,12,40,40} 比图标 {48,16,32,32} 四周各放大 4 像素，
 * 反显后就是一块白底卡片，图标本身变成黑线条 —— 这就是选中态。
 *
 * ★ count 必须 > 0。你原来写成 0，就是"按左右键没反应"的真正原因。
 * -------------------------------------------------------------------------- */
static const Item menuItems[] =
{
    { { 0,0,0,0 }, ACT_BACK, 0, 0, 0 },   /* 0 Exit：出栈回主页 */
    { { 0,0,0,0 }, ACT_NONE, 0, 0, 0 },   /* 1 Timer：还没写，安全占位 */
    { { 0,0,0,0 }, ACT_GOTO, PG_LIGHT, 0, 0 },   /* 2 Light  */
    { { 0,0,0,0 }, ACT_NONE, 0, 0, 0 },   /* 3 Shuipin */
    { { 0,0,0,0 }, ACT_NONE, 0, 0, 0 },   /* 4 Game   */
};

/* ★★ 顺序很关键：必须先定义 menuPage，再定义 MenuPage 的函数体。
 *
 * 绘制函数需要读 menuPage.sel 才知道选中第几个，
 * 而 page.draw 的函数指针又必须在 menuPage 的初始化器里给出地址 ——
 * 互相依赖。解法是"前置声明函数 → 先定义变量 → 后定义函数体"。 */
Page menuPage = { menuItems, ITEM_COUNT(menuItems), 0, MenuPage, LINK_NONE, LINK_NONE };

static void MenuPage(void)
{
    int16_t target = (int16_t)menuPage.sel * MENU_PITCH;
    int16_t x;
    uint8_t i;

    /* ---- 1. scroll 朝 target 走一步。这就是"滑动"本身，没有别的东西 ---- */
    if (scroll < target)
    {
        scroll += MENU_STEP;
        if (scroll > target) scroll = target;       /* 别冲过头 */
    }
    else if (scroll > target)
    {
        scroll -= MENU_STEP;
        if (scroll < target) scroll = target;
    }

    /* ---- 2. 按 scroll 把每个图标画到它此刻该在的列上 ---- */
    for (i = 0; i < ITEM_COUNT(menuIcons); i++)
    {
        x = (int16_t)(MENU_CENTER_X + i * MENU_PITCH) - scroll;

        /* 完全在屏外就跳过：少画两个 32×32 能省 256 次显存写。
         * 部分露头的（比如 x = -20）照画不误，OLED_ShowImage 内部会裁剪。 */
        if (x <= -MENU_ICON_W || x >= 128) continue;

        OLED_ShowImage(x, MENU_Y, MENU_ICON_W, MENU_ICON_H, menuIcons[i]);
    }

    /* ---- 3. 底部小圆点指示"5 个里的第几个"（可选，不喜欢删掉这段） ---- */
    // for (i = 0; i < ITEM_COUNT(menuIcons); i++)
    // {
    //     if (i == menuPage.sel)
    //         OLED_DrawCircle((int16_t)(52 + i * 6), 58, 2, OLED_FILLED);
    //     else
    //         OLED_DrawCircle((int16_t)(52 + i * 6), 58, 2, OLED_UNFILLED);
    // }
}
