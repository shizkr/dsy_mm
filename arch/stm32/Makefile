TCPREFIX = ~/sat/gcc-arm-none-eabi-4_7-2012q4/bin/arm-none-eabi-

CC      = $(TCPREFIX)gcc
LD      = $(TCPREFIX)ld -v
CP      = $(TCPREFIX)objcopy
OD      = $(TCPREFIX)objdump
GDBTUI  = $(TCPREFIX)gdbtui

STM32FLASH = ./stm32_flash.pl

CINCLUDE = -Ilib/stm32f4xx_stdperiph_driver/inc -Ilib/cmsis/ST/STM32F4xx/Include -Iutility \
		   -Ihw -Ilib/cmsis/Include

DEFS = -DUSE_STDPERIPH_DRIVER -DSTM32F4XX -DMANGUSTA_DISCOVERY -DUSE_USB_OTG_FS -DHSE_VALUE=8000000

# -mfix-cortex-m4-ldrd should be enabled by default for Cortex M4.
CFLAGS  =  -I. $(CINCLUDE) -fno-common -O0 -g -mcpu=cortex-m4 -mthumb -Wl,-T,stm32_flash.ld $(DEFS)
LFLAGS  = -Tstm32_flash.ld -nostartfiles
CPFLAGS = -Obinary
ODFLAGS = -S

STARTUP = utility/startup_stm32f4xx.s

UTILITY = utility/main.c \
		 utility/system_stm32f4xx.c \
		 utility/stm32f4xx_it.c
HW = hw/stm32f4_discovery.c

LIBDIR = lib/stm32f4xx_stdperiph_driver/src
LIB =   $(LIBDIR)/misc.c \
		$(LIBDIR)/stm32f4xx_can.c \
		$(LIBDIR)/stm32f4xx_crc.c \
		$(LIBDIR)/stm32f4xx_dac.c \
		$(LIBDIR)/stm32f4xx_dbgmcu.c \
		$(LIBDIR)/stm32f4xx_dcmi.c \
		$(LIBDIR)/stm32f4xx_dma.c \
		$(LIBDIR)/stm32f4xx_exti.c \
		$(LIBDIR)/stm32f4xx_flash.c \
		$(LIBDIR)/stm32f4xx_fsmc.c \
		$(LIBDIR)/stm32f4xx_gpio.c \
		$(LIBDIR)/stm32f4xx_hash.c \
		$(LIBDIR)/stm32f4xx_i2c.c \
		$(LIBDIR)/stm32f4xx_iwdg.c \
		$(LIBDIR)/stm32f4xx_pwr.c \
		$(LIBDIR)/stm32f4xx_rcc.c \
		$(LIBDIR)/stm32f4xx_rng.c \
		$(LIBDIR)/stm32f4xx_rtc.c \
		$(LIBDIR)/stm32f4xx_sdio.c \
		$(LIBDIR)/stm32f4xx_spi.c \
		$(LIBDIR)/stm32f4xx_syscfg.c \
		$(LIBDIR)/stm32f4xx_tim.c \
		$(LIBDIR)/stm32f4xx_usart.c \
		$(LIBDIR)/stm32f4xx_wwdg.c

OBJS = $(UTILITY:%.c=%.o)
OBJS += $(HW:%.c=%.o)
OBJS += $(LIB:%.c=%.o)

SRC = $(UTILITY) $(HW) $(LIB)

TARGET = main.bin
EXECUTABLE = main.elf

all: run

clean:
	-rm -f *.o $(TARGET) $(EXECUTABLE) $(EXECUTABLE:%.elf=%.map) $(EXECUTABLE:%.elf=%.lst)
	-rm -f $(EXECUTABLE:%.elf=%.out)
	-rm -f $(OBJS)

run: $(TARGET)
	$(STM32FLASH) $(TARGET)

$(TARGET): $(EXECUTABLE)
	@echo "...copying"
	$(CP) $(CPFLAGS) $(EXECUTABLE) $(TARGET)
	$(OD) $(ODFLAGS) $(EXECUTABLE)> $(EXECUTABLE:%.elf=%.lst)
	$(LD) $(ODFLAGS) $(EXECUTABLE) -M -o $(EXECUTABLE:%.elf=%.out)> $(EXECUTABLE:%.elf=%.map)

$(EXECUTABLE): $(SRC) $(STARTUP)
	@echo "..linking"
	$(CC) $(CFLAGS) $^ -o $@

$(OBJS): $(OBJS:%.o=%.c)
	@echo ".compiling"
	$(CC) $(CFLAGS) $< -o $@

debug:
	$(GDBTUI) -ex "target remote localhost:3333" \ 
		-ex "set remote hardware-breakpoint-limit 6" \ 
		-ex "set remote hardware-watchpoint-limit 4" main.elf
