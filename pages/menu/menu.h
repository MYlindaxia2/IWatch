#ifndef __MENU_H
#define __MENU_H

/* 原版是 "../stm32f10x.h" —— 从 pages/menu/ 退一级是 pages/，
 * 那里没有这个头文件；而且它用了 Page 却没 include uiExchange.h，两个必爆的错。 */
#include "../../utils/uiExchange.h"

extern Page menuPage;

#endif
