#ifndef __UPDATETIME_H
#define __UPDATETIME_H

#include "../../utils/uiExchange.h"

/* 这两个页面只导出各自的 Page */
extern Page datePage;
extern Page timePage;

void AddDateOrTime(uint8_t index);

void SubDateOrTime(uint8_t index);

#endif /* __UPDATETIME_H */
