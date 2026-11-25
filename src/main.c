/*
 * ************************************************
 *
 *              STM32 智能手表 main
 *
 *  CPU: STM32F103C8
 *
 * ************************************************
*/

#include "stm32f10x.h"
#include "../hardware/oled/OLED.h"
#include "../hardware/rtc/MyRTC.h"
#include "../hardware/delay/Delay.h"
#include "../hardware/key/Key.h"
#include "../hardware/led/LED.h"
#include "../hardware/tim/Timer.h"
#include "../pages/main/main.h"
#include "../utils/uiExchange.h"

int main()
{
    /* ------------------------------------------------------------------
     * 初始化顺序的依赖关系：
     *
     *   Timer_Init 与 Key_Init 之间没有先后要求，但两者都必须在进入主循环
     *   之前完成 —— 因为 KeyGetNumState() 的消抖计时（Key_Tick）是靠 TIM2
     *   每 1ms 中断累加的。中断没跑起来，按键永远不会被判为"稳定"，
     *   表现就是"按键完全没反应"。
     *
     *   MyRTC_Init 内部会等 LSE 起振，如果板子上没焊 32.768kHz 晶振，
     *   程序会卡死在这一行（MyRTC.c 里注释了改用 LSI 的替代版本）。
     * ------------------------------------------------------------------ */
    OLED_Init();
    MyRTC_Init();
    LED_Init();
    Key_Init();
    Timer_Init();

    /* ------------------------------------------------------------------
     * 主循环只有一件事：驱动 UI。
     *
     * 页面切换、按键响应、绘制、刷屏全都在 Ui_Task() 里面，
     * main.c 不需要知道有几个页面、当前在哪一页 ——
     * 这是把"导航状态"收进内核的好处：换 UI 结构不用改 main。
     *
     * 注意这是"每帧全量重绘"：OLED_Clear + 重画 + OLED_Update(整屏 1KB)。
     * 对 STM32F103 + 软件 I2C 来说已经吃掉不少时间，
     * 以后想省电/提响应，优化点在 Ui_Task 里加脏标记，而不是在这里加延时。
     * ------------------------------------------------------------------ */
    while (1)
    {
        Ui_Task();
    }
}
