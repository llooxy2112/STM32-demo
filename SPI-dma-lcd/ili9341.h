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

#define MADCTLGRAPHICS 0x6
#define MADCTLBMP      0x2

#define ILI9341_width  240
#define ILI9341_height 320

void ILI9341_setAddrWindow(uint16_t x0, uint16_t y0, 
			  uint16_t x1, uint16_t y1, uint8_t madctl);
void ILI9341_pushColor(uint16_t *color, int cnt);
void ILI9341_init();
void ILI9341_backLight(uint8_t on);

#endif
