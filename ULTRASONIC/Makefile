TEMPLATEROOT = ..

# compilation flags for gdb

CFLAGS  += -O0 -g
ASFLAGS += -g 
#LDLIBS += -lc -lgcc -lrdimon

# object files

OBJS=  $(STARTUP) main.o uart.o timer.o
OBJS+= stm32f10x_gpio.o stm32f10x_rcc.o stm32f10x_usart.o stm32f10x_tim.o

# include common make file

include $(TEMPLATEROOT)/Makefile.common


