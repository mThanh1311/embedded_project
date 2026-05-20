#ifndef __XPT2046_H
#define __XPT2046_H

#include <stdint.h>
#include "lcd_driver.h"

typedef char bool;
#define true 1
#define false 0

extern void xpt2046_init(void);
extern void xpt2046_read_xy(uint16_t *phwXpos, uint16_t *phwYpos);
extern bool xpt2046_twice_read_xy(uint16_t *phwXpos, uint16_t *phwYpos);
// Thêm vào XPT2046.h
extern uint16_t xpt2046_read_average(uint8_t chCmd);
extern uint16_t xpt2046_filter_read(uint8_t command);
#endif



