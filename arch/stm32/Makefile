TCPREFIX = ~/sat/gcc-arm-none-eabi-4_7-2012q4/bin/arm-none-eabi-

CC      = $(TCPREFIX)gcc
LD      = $(TCPREFIX)gcc
CP      = $(TCPREFIX)objcopy
OD      = $(TCPREFIX)objdump
GDBTUI  = $(TCPREFIX)gdbtui

TARGET = main

CORE = cortex-m4
STM32FLASH = ./stm32_flash.pl
LDSCRIPT = stm32_flash.ld

INCLUDES = -Ilib/stm32f4xx_stdperiph_driver/inc -Ilib/cmsis/ST/STM32F4xx/Include -Iutility \
		   -Ihw -Ilib/cmsis/Include

DEFS = -DUSE_STDPERIPH_DRIVER -DSTM32F4XX -DMANGUSTA_DISCOVERY -DUSE_USB_OTG_FS -DHSE_VALUE=8000000
DEFS += -DCORE=40 -DSAVE_FPU

# -mfix-cortex-m4-ldrd should be enabled by default for Cortex M4.
AFLAGS  = -mthumb -mcpu=$(CORE) $(INCLUDES)
CFLAGS  = -mthumb -mcpu=$(CORE) $(INCLUDES)
CFLAGS += -fno-common -O0 -g -mfloat-abi=soft -mfpu=fpv4-sp-d16 -fsingle-precision-constant
CFLAGS += -Wl,-T,stm32_flash.ld $(DEFS)
CFLAGS += -ffunction-sections -fdata-sections -std=gnu99 -Wall -Wno-unused -g -Os

LIBS = -lm -lc -lnosys

LDFLAGS = -mthumb -mcpu=$(CORE)
LDFLAGS += -nodefaultlibs -nostartfiles
LDFLAGS += -Wl,-Map,$(TARGET).map -T$(LDSCRIPT)
LDFLAGS += -Wl,--gc-sections

CPFLAGS = -Obinary
ODFLAGS = -S

STARTUP = utility/startup_stm32f4xx.s

UTILITY = utility/main.c \
		 utility/syscalls.c \
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

SOURCE = $(UTILITY) $(HW) $(LIB) $(STARTUP)

#---------------------------------------------------
CSOURCES = $(filter %.c,$(SOURCE))
ASMSOURCES = $(filter %.s,$(SOURCE))

OBJECTS = $(addprefix obj/, $(CSOURCES:.c=.c.o))
OBJECTS += $(addprefix obj/, $(ASMSOURCES:.s=.s.o))

DISASM = $(addprefix disasm/, $(CSOURCES:.c=.c.s)) \
          $(addprefix disasm/, $(ASMSOURCES:.s=.s.s))

LIBGCC = ${shell ${CC} ${CFLAGS} -print-libgcc-file-name}
LIBC = ${shell ${CC} ${CFLAGS} -print-file-name=libc.a}

#---------------------------------------------------
all: $(TARGET).bin

clean:
	-rm -f $(TARGET).elf $(TARGET).bin $(TARGET).map
	-rm -rf deps
	-rm -rf obj

flash: $(TARGET).bin
	$(STM32FLASH) $(TARGET).bin

$(TARGET).bin: $(TARGET).elf
	@echo "...copying"
	$(CP) $(CPFLAGS) $(TARGET).elf $(TARGET).bin

$(TARGET).elf: $(OBJECTS) $(LDSCRIPT)
	@echo "..linking"
	#$(LD) -T ${LDSCRIPT} ${LDFLAGS} -o $@ $(OBJECTS) $(LIBS) '${LIBC}' '${LIBGCC}'
	$(LD) -T ${LDSCRIPT} ${CFLAGS} -o $@ $(OBJECTS)

debug:
	$(GDBTUI) -ex "target remote localhost:3333" \ 
		-ex "set remote hardware-breakpoint-limit 6" \ 
		-ex "set remote hardware-watchpoint-limit 4" main.elf

disasm: $(DISASM)
	arm-none-eabi-objdump -d $(EXECNAME).elf >disasm/$(EXECNAME).s

flash_st:
	arm-none-eabi-objcopy -O binary $(EXECNAME).elf $(EXECNAME).bin
	st-flash write $(EXECNAME).bin 0x08000000

obj/%.c.o: %.c
	$(shell if [ ! -d $(addprefix obj/, $(dir $<)) ]; then mkdir -p $(addprefix obj/, $(dir $<)); fi )
	$(CC) $(CFLAGS) -c $< -o $@

obj/%.cpp.o: %.cpp
	$(shell if [ ! -d $(addprefix obj/, $(dir $<)) ]; then mkdir -p $(addprefix obj/, $(dir $<)); fi )
	$(CC) $(CFLAGS) -c $< -o $@

obj/%.s.o: %.s
	$(shell if [ ! -d $(addprefix obj/, $(dir $<)) ]; then mkdir -p $(addprefix obj/, $(dir $<)); fi )
	$(CC) $(CFLAGS) -c $< -o $@

disasm/%.c.s: %.c
	$(shell if [ ! -d $(addprefix disasm/, $(dir $<)) ]; then mkdir -p $(addprefix disasm/, $(dir $<)); fi )
	$(CC) $(CFLAGS) -S $< -o $@

disasm/%.cpp.s: %.cpp
	$(shell if [ ! -d $(addprefix disasm/, $(dir $<)) ]; then mkdir -p $(addprefix disasm/, $(dir $<)); fi )
	$(CC) $(CFLAGS) -S $< -o $@

disasm/%.s.s: %.s
	$(shell if [ ! -d $(addprefix disasm/, $(dir $<)) ]; then mkdir -p $(addprefix disasm/, $(dir $<)); fi )
	$(CC) $(CFLAGS) -S $< -o $@

#----------------------------------------------------------
deps/%.c.d: %.c
	$(shell if [ ! -d $(addprefix deps/, $(dir $<)) ]; then mkdir -p $(addprefix deps/, $(dir $<)); fi )
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

deps/%.cpp.d: %.cpp
	$(shell if [ ! -d $(addprefix deps/, $(dir $<)) ]; then mkdir -p $(addprefix deps/, $(dir $<)); fi )
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

deps/%.s.d: %.s
	$(shell if [ ! -d $(addprefix deps/, $(dir $<)) ]; then mkdir -p $(addprefix deps/, $(dir $<)); fi )
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
