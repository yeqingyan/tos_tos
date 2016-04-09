# The platform can be raspi or qemu
PLATFORM := qemu

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
OBJECTS := build/dispatch.o build/gpio.o build/main.o build/mem.o build/process.o build/stdlib.o build/systemTimer.o  build/start.o build/intr.o build/irq_handler.o build/framebuffer.o build/mailbox.o build/drawing.o build/font.o build/pacman.o build/window.o build/usb.o build/ipc.o build/assert.o
#OBJECTS := $(patsubst $(SOURCE)%.c,$(BUILD)%.o,$(wildcard $(SOURCE)*.c)) $(patsubst $(SOURCE)%.s, $(BUILD)%.o, $(wildcard $(SOURCE)*.s))

# Copy platform files
PLATFORM_FILES = include/timer.h include/intr.h source/intr.c

# Boot address for Raspberry Pi is 0x8000
# Boot address for QEMU is 0x10000
ifeq ($(PLATFORM), qemu)
	BOOT_ADDRESS = 0x10000
else ifeq ($(PLATFORM), raspi)
	BOOT_ADDRESS = 0x8000
endif

# Rule to make everything.
all: $(TARGET) $(LIST)

# Rule to remake everything. Does not include clean.
rebuild: all

# Rule to make the listing file.
$(LIST) : $(BUILD)output.elf
	$(ARMGNU)-objdump -S -d $(BUILD)output.elf > $(LIST)

# Rule to make the image file.
$(TARGET) : $(BUILD)output.elf
	$(ARMGNU)-objcopy $(BUILD)output.elf -O binary $(TARGET) 

# Rule to make the elf file.
# TODO add ,--no-wchar-size-warning if no error happened
$(BUILD)output.elf : $(OBJECTS)
	$(ARMGNU)-gcc $(GCCARGS) $(OBJECTS) -o $(BUILD)output.elf -Wl,-L,.,-l,csud,--section-start,.init=$(BOOT_ADDRESS),--section-start,.stack=0xA0000,-Map=$(MAP)

# Rule to make the object files.
# Note by Yeqing:
# Gcc using -O2 or -O3 sometimes got problems, if code runs not as expected, try turn off -O first 
$(BUILD)%.o: $(SOURCE)%.c $(BUILD)
	$(ARMGNU)-gcc $(GCCARGS) -c $< -o $@

$(BUILD)%.o: $(SOURCE)%.s $(BUILD)
	$(ARMGNU)-as -I./include $< -o $@
    
# Generate object file from binary    
build/font.o : font/font.bin
	$(ARMGNU)-objcopy -I binary -O elf32-littlearm -B arm font/font.bin build/font.o

$(BUILD):
	mkdir $@

# Rule to clean files.
clean : 
	-rm -rf $(BUILD)
	-rm -f $(TARGET)
	-rm -f $(LIST)
	-rm -f $(MAP)
