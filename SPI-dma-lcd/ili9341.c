/*************************************************** 
  This is a library for the Adafruit 1.8" SPI display.
  This library works with the Adafruit 1.8" TFT Breakout w/SD card  
  ----> http://www.adafruit.com/products/358  
  as well as Adafruit raw 1.8" TFT display  
  ----> http://www.adafruit.com/products/618
 
  Check out the links above for our tutorials and wiring diagrams 
  These displays use SPI to communicate, 4 or 5 pins are required to  
  interface (RST is optional) 
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  MIT license, all text above must be included in any redistribution
 ****************************************************/

/* 
 The code contained here is based upon the adafruit driver, but heavily modified
*/

#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include "spidma.h"
#include "tm_stm32f4_fonts.h"
#include "ili9341.h"
#include "glcdfont.c"


#define LCD_PORT_BKL  GPIOA
#define LCD_PORT GPIOA
#define GPIO_PIN_BKL GPIO_Pin_3
#define GPIO_PIN_RST GPIO_Pin_1
#define GPIO_PIN_SCE GPIO_Pin_0
#define GPIO_PIN_DC GPIO_Pin_2

#define LCDSPEED SPI_BaudRatePrescaler_2

#define LOW  0
#define HIGH 1

#define LCD_C LOW
#define LCD_D HIGH

#define ILI9341_CASET 0x2A
#define ILI9341_RASET 0x2B
#define ILI9341_MADCTL 0x36
#define ILI9341_RAMWR 0x2C
#define ILI9341_RAMRD 0x2E
#define ILI9341_COLMOD 0x3A

#define ILI9341_MADCTL_START 0xE8

#define MADVAL(x) (((x) << 5) | 8)
static uint8_t madctlcurrent = ILI9341_MADCTL_START;

struct ILI9341_cmdBuf {
  uint8_t command;   // ILI9341 command byte
  uint8_t delay;     // ms delay after
  uint8_t len;       // length of parameter data
  uint8_t data[16];  // parameter data
};

/* Pin functions */
uint16_t ILI9341_x;
uint16_t ILI9341_y;


static const struct ILI9341_cmdBuf initializers[] = {
  // SWRESET Software reset 
  { 0x01, 150, 0, 0},
  // SLPOUT Leave sleep mode
  { 0x11,  150, 0, 0},
  // FRMCTR1, FRMCTR2 Frame Rate configuration -- Normal mode, idle
  // frame 
  { 0xB1, 0, 2, { 0x00, 0x1B }}, // 70 Hz default
  { 0xB2, 0, 2, { 0x00, 0x1B }},
  // FRMCTR3 Frame Rate configureation -- partial mode
  { 0xB3, 0, 2, { 0x00, 0x1B }},
  // INVCTR Display inversion (no inversion)
  { 0xB4,  0, 1, { 0x00 }},
  // PWCTR1 Power control -4.6V, Auto mode
  { 0xC0,  0, 1, { 0x23}},
  // PWCTR2 Power control VGH25 2.4C, VGSEL -10, VGH = 3 * AVDD
  { 0xC1,  0, 1, { 0x03}},
  // PWCTR3 Power control, opamp current smal, boost frequency
  { 0xC2,  0, 2, { 0x0A, 0x00 }},
  // PWCTR4 Power control, BLK/2, opamp current small and medium low
  { 0xC3,  0, 2, { 0x8A, 0x2A}},
  // INVOFF Don't invert display
  { 0x20,  0, 0, 0},
  // Memory access directions. row address/col address, bottom to top refesh (10.1.27)
  { ILI9341_MADCTL,  0, 1, { ILI9341_MADCTL_START }},
  // Color mode 16 bit (10.1.30
  { ILI9341_COLMOD,   0, 1, {0x55}},
  // Column address set 0..239
  // { ILI9341_CASET,   0, 4, {0x00, 0x00, 0x00, 0xEF }},
  // Row address set 0..319
  // { ILI9341_RASET,   0, 4, {0x00, 0x00, 0x01, 0x3F }},
  // GMCTRP1 Gamma correction
  { 0xE0, 0, 15, {0x0F, 0x32, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
			    0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00 }},
  // GMCTRP2 Gamma Polarity corrction
  { 0xE1, 0, 15, {0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
			    0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F }},
  // DISPON Display on
  { 0x29, 100, 0, 0},
  // NORON Normal on
  { 0x13,  10, 0, 0},
  // End
  { 0, 0, 0, 0}
};

static void LcdWrite(char dc, const char *data, int cnt) {
  GPIO_WriteBit(LCD_PORT,GPIO_PIN_DC, dc);  // dc 1 = data, 0 = control
  GPIO_ResetBits(LCD_PORT,GPIO_PIN_SCE);
  spi_read_write(SPILCD, 0, data, cnt, LCDSPEED);
  GPIO_SetBits(LCD_PORT,GPIO_PIN_SCE); 
}

static void LcdWrite16(char dc, const uint16_t *data, int cnt) {
  GPIO_WriteBit(LCD_PORT,GPIO_PIN_DC, dc);  // dc 1 = data, 0 = control
  GPIO_ResetBits(LCD_PORT,GPIO_PIN_SCE);
  spi_read_write16(SPILCD, 0, data, cnt, LCDSPEED);
  GPIO_SetBits(LCD_PORT,GPIO_PIN_SCE); 
}

static void ILI9341_writeCmd(uint8_t c) {
  LcdWrite(LCD_C, &c, 1);
}

void ILI9341_setAddrWindow(uint16_t x0, uint16_t y0, 
			  uint16_t x1, uint16_t y1, uint8_t madctl) {
/*
  // madctl = MADVAL(madctl);
  if (madctl != madctlcurrent){
      ILI9341_writeCmd(ILI9341_MADCTL);
      LcdWrite(LCD_D, &madctl, 1);
      madctlcurrent = madctl;
  }
*/
  ILI9341_writeCmd(ILI9341_CASET);
  LcdWrite16(LCD_D, &x0, 1);
  LcdWrite16(LCD_D, &x1, 1);

  ILI9341_writeCmd(ILI9341_RASET);
  LcdWrite16(LCD_D, &y0, 1);
  LcdWrite16(LCD_D, &y1, 1);

  ILI9341_writeCmd(ILI9341_RAMWR);
}

void ILI9341_pushColor(uint16_t *color, int cnt) {
  LcdWrite16(LCD_D, color, cnt);
}

void ILI9341_backLight(uint8_t on) {
   GPIO_WriteBit(LCD_PORT_BKL,GPIO_PIN_BKL, (on) ? Bit_SET : Bit_RESET);
}

void ILI9341_putchar(uint8_t x,uint8_t y,uint8_t ascii, uint16_t fgc, uint16_t bgc) {
	uint16_t cbuf[35]; // 5x7 pixel
	uint8_t i,j,bitmask,fontbits;
	uint16_t pos;
	
	pos = ascii*5;
	for (i=0;i<5;i++) {
		fontbits = font[pos+i];
		for (j=0;j<7;j++) {
			bitmask = 1 << (6-j);
			if ((fontbits&bitmask)) {
				cbuf[i*7+j] = fgc;
			} else {
				cbuf[i*7+j] = bgc;
			}
		}
	}

	ILI9341_setAddrWindow((uint16_t) x*8, (uint16_t) y*6, (uint16_t) x*8+6, (uint16_t) y*6+4, madctlcurrent);
	GPIO_WriteBit(LCD_PORT,GPIO_PIN_DC, LCD_D);  // dc 1 = data, 0 = control
	GPIO_ResetBits(LCD_PORT,GPIO_PIN_SCE);
	spi_read_write16(SPILCD, 0, cbuf, 35, LCDSPEED);
	GPIO_SetBits(LCD_PORT,GPIO_PIN_SCE); 
}


void ILI9341_Putc(uint16_t x, uint16_t y, char c, TM_FontDef_t *font, uint32_t foreground, uint32_t background) {
	uint32_t i, b, j;
	uint16_t LCDbuffer[512];
	//uint32_t lcd;

    /* Set coordinates */
	ILI9341_x = x;
	ILI9341_y = y;
	if ((ILI9341_x + font->FontWidth) > ILI9341_width) {
		//If at the end of a line of display, go to new line and set x to 0 position
		ILI9341_y += font->FontHeight;
		ILI9341_x = 0;
	}
	for (i = 0; i < font->FontHeight; i++) {
		b = font->data[(c - 32) * font->FontHeight + i];

		for (j = 0; j < font->FontWidth; j++) {
			if ((b << j) & 0x8000) {
				LCDbuffer[j+(i*font->FontWidth)]=((foreground << 8)|(foreground >> 8));
			} else if ((background & ILI9341_TRANSPARENT) == 0) {
				LCDbuffer[j+(i*font->FontWidth)]=((background << 8)|(background >> 8));
			}
		}
	}

	ILI9341_setAddrWindow((uint16_t) ILI9341_x, (uint16_t) ILI9341_y, (uint16_t) ILI9341_x + font->FontWidth-1, (uint16_t) ILI9341_y+font->FontHeight-1, madctlcurrent);
	GPIO_WriteBit(LCD_PORT,GPIO_PIN_DC, LCD_D);  // dc 1 = data, 0 = control
	GPIO_ResetBits(LCD_PORT,GPIO_PIN_SCE);
	spi_read_write16(SPILCD, 0, LCDbuffer, (font->FontWidth*font->FontHeight), LCDSPEED);
	GPIO_SetBits(LCD_PORT,GPIO_PIN_SCE); 

	ILI9341_x += font->FontWidth;
}

void ILI9341_Puts(uint16_t x, uint16_t y, char *str, TM_FontDef_t *font, uint32_t foreground, uint32_t background) {
	uint16_t startX = x;

	/* Set X and Y coordinates */
	ILI9341_x = x;
	ILI9341_y = y;

	while (*str) {
		//New line
		if (*str == '\n') {
			ILI9341_y += font->FontHeight + 1;
			//if after \n is also \r, than go to the left of the screen
			if (*(str + 1) == '\r') {
				ILI9341_x = 0;
				str++;
			} else {
				ILI9341_x = startX;
			}
			str++;
			continue;
		} else if (*str == '\r') {
			str++;
			continue;
		}

		ILI9341_Putc(ILI9341_x, ILI9341_y, *str++, font, foreground, background);
	}
}

void ILI9341_dup_putchar(uint16_t x,uint16_t y,uint8_t ascii, uint16_t fgc, uint16_t bgc) {
	uint16_t cbuf[140]; // 5x7x4 pixel
	uint8_t i,j,bitmask,fontbits;
	uint16_t pos;
	
	pos = ascii*5;
	for (i=0;i<5;i++) {
		fontbits = font[pos+i];
		for (j=0;j<7;j++) {
			bitmask = 1 << (6-j);
			if ((fontbits&bitmask)) {
				cbuf[i*28+j*2] = fgc;
				cbuf[i*28+j*2+1] = fgc;
				cbuf[i*28+j*2+14] = fgc;
				cbuf[i*28+j*2+15] = fgc;
			} else {
				cbuf[i*28+j*2] = bgc;
				cbuf[i*28+j*2+1] = bgc;
				cbuf[i*28+j*2+14] = bgc;
				cbuf[i*28+j*2+15] = bgc;
			}
		}
	}

	ILI9341_setAddrWindow((uint16_t) x*16, (uint16_t) y*12, (uint16_t) x*16+13, (uint16_t) y*12+9, madctlcurrent);
	GPIO_WriteBit(LCD_PORT,GPIO_PIN_DC, LCD_D);  // dc 1 = data, 0 = control
	GPIO_ResetBits(LCD_PORT,GPIO_PIN_SCE);
	spi_read_write16(SPILCD, 0, cbuf, 140, LCDSPEED);
	GPIO_SetBits(LCD_PORT,GPIO_PIN_SCE); 
}

void ILI9341_dup_print_col(uint8_t *cbuf, uint16_t x,uint16_t y, uint16_t fgc, uint16_t bgc) {

	uint16_t ix,iy;
	ix = x;
	iy = y;

	while (*cbuf && ix>0) {
		ILI9341_dup_putchar(ix,iy,*cbuf++, fgc+ix*iy, bgc);
		iy++;
		if (iy>25) {
			iy=0;
			ix--;
		}
	}
}

void ILI9341_dup_print(uint8_t *cbuf, uint16_t x,uint16_t y, uint16_t fgc, uint16_t bgc) {

	uint16_t ix,iy;
	ix = x;
	iy = y;

	while (*cbuf && ix>0) {
		ILI9341_dup_putchar(ix,iy,*cbuf++, fgc, bgc);
		iy++;
		if (iy>25) {
			iy=0;
			ix--;
		}
	}
}

void ILI9341_fillscreen(uint16_t color) {
	uint16_t x,y;
	ILI9341_setAddrWindow(0,0,319,239,madctlcurrent);
	for (x=0;x<320;x++) {
		for (y=0;y<240;y++) {
			ILI9341_pushColor(&color,1);
		}
	}

}


void ILI9341_init() {
  GPIO_InitTypeDef GPIO_InitStructure;
  const struct ILI9341_cmdBuf *cmd;

   if (LCD_PORT_BKL==GPIOA || LCD_PORT==GPIOA) {
   	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
   } else if (LCD_PORT_BKL==GPIOB || LCD_PORT==GPIOB) {
   	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
   } else if (LCD_PORT_BKL==GPIOC || LCD_PORT==GPIOC) {
   	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
   }

   GPIO_StructInit(&GPIO_InitStructure);

   GPIO_InitStructure.GPIO_Pin = GPIO_PIN_BKL;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(LCD_PORT_BKL, &GPIO_InitStructure);

   GPIO_InitStructure.GPIO_Pin = GPIO_PIN_RST;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(LCD_PORT, &GPIO_InitStructure);

   GPIO_InitStructure.GPIO_Pin = GPIO_PIN_SCE;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(LCD_PORT, &GPIO_InitStructure);

   GPIO_InitStructure.GPIO_Pin = GPIO_PIN_DC;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(LCD_PORT, &GPIO_InitStructure);

   spi_init(SPILCD);
	  
  // set cs, reset low

  GPIO_WriteBit(LCD_PORT,GPIO_PIN_SCE, HIGH);
  GPIO_WriteBit(LCD_PORT,GPIO_PIN_RST, HIGH);
  Delay(10);
  GPIO_WriteBit(LCD_PORT,GPIO_PIN_RST, LOW);
  Delay(10);
  GPIO_WriteBit(LCD_PORT,GPIO_PIN_RST, HIGH);
  Delay(10);

  // Send initialization commands to ILI9341

  for (cmd = initializers; cmd->command; cmd++) {
      LcdWrite(LCD_C, &(cmd->command), 1);
      if (cmd->len) {
        LcdWrite(LCD_D, cmd->data, cmd->len);
      }
      if (cmd->delay) {
        Delay(cmd->delay);
      }
    }
}

