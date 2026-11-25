#ifndef __KEY_H
#define __KEY_H
#include <stdint.h>
typedef enum
{
	NONE = 0,
	ENTER = 1,
	LEFT = 2,
	RIGHT = 3
}KeyFun;
void Key_Init(void);
uint8_t Key_GetNum(void);
KeyFun KeyGetNumState(void);		/* 返回类型必须与 Key.c 一致（enum，不是 uint8_t） */
#endif
