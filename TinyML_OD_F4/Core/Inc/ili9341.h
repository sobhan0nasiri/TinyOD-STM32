#ifndef __ILI9341_H
#define __ILI9341_H

#include "stm32f4xx_hal.h"


#define ILI9341_CYAN        0x07FF
#define ILI9341_DARKGREY    0x7BEF
#define ILI9341_BLACK       0x0000
#define ILI9341_WHITE       0xFFFF
#define ILI9341_RED         0xF800
#define ILI9341_GREEN       0x07E0
#define ILI9341_YELLOW      0xFFE0

void ILI9341_Init(void);
void ILI9341_FillScreen(uint16_t color);
void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ILI9341_DrawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg);
void ILI9341_WriteString(uint16_t x, uint16_t y, const char* str, uint16_t color, uint16_t bg);

#endif