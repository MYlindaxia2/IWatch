#include "../../hardware/oled/OLED.h"
#include "../../hardware/rtc/MyRTC.h"
#include "../../utils/uiExchange.h"
#include "../edit/edit.h"
#include "../updateDate/updateTime.h"
#include "../menu/menu.h"
#include "../light/light.h"
#include "main.h"

/* ============================================================================
 * 一个页面文件长什么样？就三样东西，不多不少：
 *
 *   1. 一个 static 的绘制函数   —— 只画静态内容，不画高亮、不改任何状态
 *   2. 一张 static 的 Item 表   —— 选项的坐标 + 按下去做什么，纯数据
 *   3. 一个对外的 Page 变量     —— 把上面两样打包，给 pages[] 登记
 *
 * 不需要"进入页面/退出页面"的函数，也不需要在这里判断"当前是不是主页"。
 * 因为绘制由 Ui_Draw 调度、输入由 Ui_Input 调度，页面自己只负责"我是谁"。
 * ========================================================================== */

/* ----------------------------------------------------------------------------
 * 绘制函数
 *
 * 只画"这一页永远不变的部分"（时间和两个按钮的文字）。
 * "现在选中哪一个"由 Ui_Draw 统一反显，所以这里绝对不要写高亮逻辑。
 * 也不要在这里改 curId —— 绘制是只读操作，一旦它会改状态，
 * 就会出现"画一帧跳一页"的诡异现象。
 * -------------------------------------------------------------------------- */
static void ShowTime(void)
{
    /* MyRTC_Time 只是个缓存，必须先读硬件再取值，否则显示的是上一帧的快照 */
    MyRTC_ReadTime();

    OLED_Printf(0, 0, OLED_6X8, "%04d-%02d-%02d",
                MyRTC_Time[0], MyRTC_Time[1], MyRTC_Time[2]);
    OLED_Printf(0, 20, OLED_24X24, "%02d:%02d", MyRTC_Time[3], MyRTC_Time[4]);
    OLED_ShowString(0,  50, "菜单", OLED_8X16);
    OLED_ShowString(96, 50, "设置", OLED_8X16);
}

/* ----------------------------------------------------------------------------
 * 选项表：主页的两个入口
 *
 * 字段顺序：hot（反显区域）, act（按下去干什么）, target（去哪页）, call, arg
 *
 *   ACT_GOTO + PG_EDIT = 进设置页
 *   ACT_GOTO + PG_MENU = 想进菜单页，但 pages[PG_MENU] 还没填。
 *                        内核对 NULL 页会直接忽略，所以按下去没反应、也不会崩。
 * -------------------------------------------------------------------------- */
static const Item mainItems[] =
{
    { {  0, 50, 32, 16 }, ACT_GOTO, PG_MENU, 0, 0 },   /* 菜单 */
    { { 96, 50, 32, 16 }, ACT_GOTO, PG_EDIT, 0, 0 },   /* 设置 */
};

/* 把"选项表 + 数量 + 初始选中项 + 绘制函数 + 越界出口"打包成一个 Page。
 * count 用 ITEM_COUNT 算出来，以后增删选项不用手工改数字（改漏了会越界）。
 * 主页左右到头就环绕，所以两个 Link 都是 LINK_NONE。 */
Page mainPage = { mainItems, ITEM_COUNT(mainItems), 0, ShowTime, LINK_NONE, LINK_NONE };

/* ============================================================================
 * 页面表：整个 UI 只有这一张
 *
 * 每个页面的 .c 导出自己的 Page，这里只做"接线"（登记地址）。
 * 谁跳到谁不写在这里 —— 那属于各页面自己的 Item.target 和 leftTo/rightTo，
 * 本文件不需要知道页面之间怎么连。
 *
 * 加一个新页面就三步：
 *   1. 新建 pages/xxx/xxx.c，导出 Page xxxPage
 *   2. uiExchange.h 的 PageId 里加一个 PG_XXX
 *   3. 这张表里加一行 [PG_XXX] = &xxxPage
 *
 * 没填的位置自动是 NULL，内核会跳过 —— 所以可以边写边跑，不用一次写完。
 * ========================================================================== */
Page *pages[PG_COUNT] =
{
    [PG_MAIN] = &mainPage,
    [PG_EDIT] = &editPage,
    [PG_DATE] = &datePage,
    [PG_TIME] = &timePage,
    [PG_MENU] = &menuPage,
    [PG_LIGHT] = &lightPage,
    /* [PG_MENU] = &menuPage,   ← 菜单页写好后在这里补一行 */
};
