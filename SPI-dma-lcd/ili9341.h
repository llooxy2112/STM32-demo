#ifndef ILI9341_H
#define ILI9341_H

/*  Colors are 565 RGB (5 bits Red, 6 bits green, 5 bits blue) */

#define BLACK           0x0000
#define BLUE            0x001F
#define GREEN           0x07E0
#define CYAN            0x07FF
#define RED             0xF800
#define MAGENTA         0xF81F       
#define YELLOW          0xFFE0  
#define WHITE           0xFFFF

#define SPILCD SPI2

/* MADCTL [MY MX MV]
 *    MY  row address order   1 (bottom to top), 0 (top to bottom)
 *    MX  col address order   1 (right to left), 0 (left to right)
 *    MV  col/row exchange    1 (exchange),      0 normal
 */

/* Transparent background, only for strings and chars */
#define ILI9341_TRANSPARENT			0x80000000

#define MADCTLGRAPHICS 0x6
#define MADCTLBMP      0x2

#define ILI9341_width  320
#define ILI9341_height 240

void ILI9341_setAddrWindow(uint16_t x0, uint16_t y0, 
			  uint16_t x1, uint16_t y1, uint8_t madctl);
void ILI9341_pushColor(uint16_t *color, int cnt);
void ILI9341_init();
void ILI9341_backLight(uint8_t on);
void ILI9341_dup_print(uint8_t *cbuf, uint16_t x,uint16_t y, uint16_t fgc, uint16_t bgc);
void ILI9341_print(uint8_t *cbuf, uint16_t x,uint16_t y, uint16_t fgc, uint16_t bgc);
void ILI9341_Putc(uint16_t x, uint16_t y, char c, TM_FontDef_t *font, uint32_t foreground, uint32_t background);
void ILI9341_Puts(uint16_t x, uint16_t y, char *str, TM_FontDef_t *font, uint32_t foreground, uint32_t background);


#endif
