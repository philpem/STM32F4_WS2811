.PHONY: all all_real clean

# Prevent included makefiles from overriding the default rules
all: all_real

# Set the target platform
PLATFORM := STM32F40_41xxx

# Enable debugging (assert checking) for the Standard Peripheral Library. Requires a "assert_failed" function.
#STM_STDPERIPH_DEBUG := yes

# Use newlib
USE_NEWLIB := yes

# Target file names
TARGETELF=output.elf
TARGETBIN=output.bin
TARGETMAP=output.map

# Object files used to build the application
OBJS=$(addprefix obj/,startup.o system_stm32f4xx.o stm32f4xx_it.o main.o)

###
# Toolchain configuration
###

include ToolchainCfg.mk

ifeq ($(RELEASE),true)
   CFLAGS += -Wall -O3
else
   CFLAGS += -Wall -g -ggdb
endif
LDFLAGS += -nostartfiles

ifneq ($(USE_NEWLIB),yes)
  LDFLAGS += -nostdlib
endif

CFLAGS += -I./inc -I../../Utilities/STM32F4-Discovery/

# Libraries
include vendor/ST/STM32-StdPeriph/ST_STM32StdPeriph.mk

ifeq ($(USE_NEWLIB),yes)
  OBJS += obj/newlib_stubs.o
endif

# Output a map file if requested
ifneq ($(TARGETMAP),)
  LDFLAGS += -Wl,-M=$(TARGETMAP)
endif

###
# Real make rules start here
###

# Set 'quiet' flag if VERBOSE isn't set
ifeq ($(VERBOSE),)
   Q=@
endif

all_real: $(TARGETELF) $(TARGETBIN)

realclean: vendor_st_stm32_stdperiph_clean clean

clean:
	-rm obj/*.o $(TARGETELF) $(TARGETBIN)

$(TARGETELF): $(OBJS) $(SPL_LIB)
	@echo "   LD      $@"
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lgcc

$(TARGETBIN): $(TARGETELF)
	@echo "   OBJCOPY $^ => $@"
	$(Q)$(OBJCOPY) -O binary $^ $@

$(OBJS): | obj

obj:
	@echo "   MKDIR   $@"
	@mkdir -p $@
	@touch $@/.keep

obj/%.o: src/%.c
	@echo "   CC      $^"
	$(Q)$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

obj/startup.o: vendor/ST/STM32-StdPeriph/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f40_41xxx.s
	@echo "   CC      $^"
	$(Q)$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

.PHONY: debug xdebug
debug: $(TARGETELF) run.gdb
	openocd &>/dev/null &
	-arm-none-eabi-gdb -x run.gdb
	killall openocd

xdebug: $(TARGETELF) run.gdb
	openocd &>/dev/null &
	-ddd --debugger arm-none-eabi-gdb -x run.gdb
	killall openocd

run.gdb:
	@echo "targ ext :3333" > $@
	@echo "file $(TARGETELF)" >> $@
	@echo "monitor init" >> $@
	@echo "monitor reset init" >> $@
	@echo "monitor flash write_image erase $(TARGETELF)" >> $@
	@echo "monitor verify_image $(TARGETELF)" >> $@
	@echo "#load $(TARGETELF)" >> $@
	@echo "kill" >> $@
	@echo "break main" >> $@
	@echo "run" >> $@

