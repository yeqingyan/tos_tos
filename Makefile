# The platform can be raspi or qemu
PLATFORM := raspi

ARMGNU ?= arm-none-eabi

GCCARGS = -Wall -nostdinc -I./include -g -nostartfiles -fomit-frame-pointer -fno-defer-pop -mcpu=arm1176jzf-s
# The intermediate directory for compiled object files.
BUILD = build/

# The directory in which source files are stored.
SOURCE = source/

# The name of the output file to generate.
TARGET = kernel.img

# The name of the assembler listing file to generate.
LIST = kernel.list

# The name of the map file to generate.
MAP = kernel.map

# The names of all object files
OBJECTS := build/dispatch.o build/gpio.o build/main.o build/mem.o build/process.o build/stdlib.o build/systemTimer.o  build/start.o build/intr.o build/irq_handler.o
#OBJECTS := $(patsubst $(SOURCE)%.c,$(BUILD)%.o,$(wildcard $(SOURCE)*.c)) $(patsubst $(SOURCE)%.s, $(BUILD)%.o, $(wildcard $(SOURCE)*.s))

# Copy platform files
PLATFORM_FILES = include/timer.h include/intr.h source/intr.c

# Boot address for Raspberry Pi is 0x8000
# Boot address for QEMU is 0x10000
ifeq ($(PLATFORM), qemu)
	BOOT_ADDRESS = 0x10000
else ifeq ($(PLATFORM), raspi)
	BOOT_ADDRESS = 0x10000
endif

# Rule to make everything.
all: $(PLATFORM_FILES) $(TARGET) $(LIST)

# Copy platform files
$(PLATFORM_FILES):
	echo "Copy platform files"
	cp -v platforms/$(PLATFORM)/timer.h include/timer.h
	cp -v platforms/$(PLATFORM)/intr.h include/intr.h
	cp -v platforms/$(PLATFORM)/intr.c source/intr.c

# Rule to remake everything. Does not include clean.
rebuild: all

# Rule to make the listing file.
$(LIST) : $(BUILD)output.elf
	$(ARMGNU)-objdump -S -d $(BUILD)output.elf > $(LIST)

# Rule to make the image file.
$(TARGET) : $(BUILD)output.elf
	$(ARMGNU)-objcopy $(BUILD)output.elf -O binary $(TARGET) 

# Rule to make the elf file.
$(BUILD)output.elf : $(OBJECTS)
	$(ARMGNU)-gcc $(GCCARGS) $(OBJECTS) -o $(BUILD)output.elf -Wl,--section-start,.init=$(BOOT_ADDRESS),--section-start,.stack=0xA0000,-Map=$(MAP)

# Rule to make the object files.
# Note by Yeqing:
# Gcc using -O2 or -O3 sometimes got problems, if code runs not as expected, try turn off -O first 
$(BUILD)%.o: $(SOURCE)%.c $(BUILD)
	$(ARMGNU)-gcc $(GCCARGS) -c $< -o $@

$(BUILD)%.o: $(SOURCE)%.s $(BUILD)
	$(ARMGNU)-as -I./include $< -o $@

$(BUILD):
	mkdir $@

# Rule to clean files.
clean : 
	-rm -rf $(BUILD)
	-rm -f $(TARGET)
	-rm -f $(LIST)
	-rm -f $(MAP)
	-rm -f include/timer.h
	-rm -f include/intr.h
	-rm -f source/intr.c
