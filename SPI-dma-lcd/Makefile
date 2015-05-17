TEMPLATEROOT = ..

# compilation flags for gdb

CFLAGS  += -O0 -g
ASFLAGS += -g 
#LDLIBS += -lc -lgcc -lrdimon

# object files

OBJS=  $(STARTUP) main.o uartdma.o spidma.o ili9341.o xprintf.o tm_stm32f4_fonts.o
OBJS+= stm32f10x_gpio.o stm32f10x_rcc.o stm32f10x_usart.o stm32f10x_spi.o stm32f10x_dma.o

# include common make file

include $(TEMPLATEROOT)/Makefile.common


