#include "../../hardware/oled/OLED.h"
#include "../../hardware/rtc/MyRTC.h"
#include "../../utils/uiExchange.h"
#include "updateTime.h"

/* ============================================================================
 * 日期页（年 / 月 / 日）和时间页（时 / 分 / 秒）
 *
 * 核心思路：把方向键想象成在一条"连续长列表"上移动，页面只是显示窗口。
 * 光标走到日期的最后一项还继续按，就不该是"环绕回顶部"，而是翻到下一页：
 *
 *       返回 → 年 → 月 → 日 │ 时 → 分 → 秒
 *                          └──翻页──┘
 *
 * 这条链是靠 Page 里的 leftTo / rightTo 两个"越界出口"连起来的，
 * Ui_Input 里一行 if 都不用为这两个页面单独写。
 * ========================================================================== */

/* 两个绘制函数只负责把当前时间摆到屏幕上。
 * 高亮（选中哪一行）由 Ui_Draw 用 items[sel].hot 统一反显，这里不画。 */
static void UpdateDatePage(void)
{
    MyRTC_ReadTime();
    OLED_ShowString(0,  0, "返回", OLED_8X16);
    OLED_Printf(0, 16, OLED_8X16, "年:%04d", MyRTC_Time[0]);
    OLED_Printf(0, 32, OLED_8X16, "月:%02d", MyRTC_Time[1]);
    OLED_Printf(0, 48, OLED_8X16, "日:%02d", MyRTC_Time[2]);
}

static void UpdateTimePage(void)
{
    MyRTC_ReadTime();
    OLED_Printf(0,  0, OLED_8X16, "时:%02d", MyRTC_Time[3]);
    OLED_Printf(0, 16, OLED_8X16, "分:%02d", MyRTC_Time[4]);
    OLED_Printf(0, 32, OLED_8X16, "秒:%02d", MyRTC_Time[5]);
}

/* ----------------------------------------------------------------------------
 * 选项表
 *
 * 最后那个字段 arg 是"这一个选项对应哪个时间字段"，
 * 它正好就是 MyRTC_Time 的下标：
 *
 *     0=年  1=月  2=日  3=时  4=分  5=秒
 *
 * 这样两个页面可以共用一个 Field_Add(field) 函数，
 * 不需要写 Year_Add / Month_Add / Day_Add 三个几乎一样的函数。
 * 这也是"差异数据化"的好处：行为相同、参数不同，就只写一份逻辑。
 *
 * 现在 ACT_EDIT 还没接实现，所以按 ENTER 暂时没反应（内核 switch 走 default）。
 * -------------------------------------------------------------------------- */
static const Item dateItems[] =
{
    { { 0,  0, 32, 16 }, ACT_BACK, 0, 0, 0 },     /* 返回：出栈回设置页 */
    { { 0, 16, 16, 16 }, ACT_EDIT, 0, 0, 0 },     /* 年 */
    { { 0, 32, 16, 16 }, ACT_EDIT, 0, 0, 1 },     /* 月 */
    { { 0, 48, 16, 16 }, ACT_EDIT, 0, 0, 2 },     /* 日 */
};

static const Item timeItems[] =
{
    { { 0,  0, 16, 16 }, ACT_EDIT, 0, 0, 3 },     /* 时 */
    { { 0, 16, 16, 16 }, ACT_EDIT, 0, 0, 4 },     /* 分 */
    { { 0, 32, 16, 16 }, ACT_EDIT, 0, 0, 5 },     /* 秒 */
};

/* ----------------------------------------------------------------------------
 * 两个页面的"越界出口" —— 这里就是整本书的装订线
 *
 * datePage：
 *   leftTo  = { PG_TIME, 0 }  在"日"(最后一项)继续按左 → 时间页第 0 项（时）
 *   rightTo = LINK_NONE       在"返回"(第一项)按右 → 环绕到"日"
 *
 * timePage：
 *   leftTo  = { PG_DATE, 0 }  在"秒"(最后一项)继续按左 → 日期页"返回"
 *   rightTo = { PG_DATE, 3 }  在"时"(第一项)按右 → 日期页第 3 项（日）★
 *
 * 注意 rightTo 的落点 3 是写在"跳转"上的，不是写在目标页上的。
 * 如果落点存在 datePage 里（以前那种 enterSel），它就只能有一个值，
 * 那么"从时间页翻回来落在日"和"从设置页进来落在返回"必然冲突。
 * -------------------------------------------------------------------------- */
Page datePage = { dateItems, ITEM_COUNT(dateItems), 0, UpdateDatePage,
                  { PG_TIME, 0 }, LINK_NONE };

Page timePage = { timeItems, ITEM_COUNT(timeItems), 0, UpdateTimePage,
                  { PG_DATE, 0 }, { PG_DATE, 3 } };
