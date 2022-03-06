BUILD_DIR = ./build
# Lib
LIB_SRC_DIR = ../lib

LIB_SRC = \
	$(LIB_SRC_DIR)/cpio.c \
	$(LIB_SRC_DIR)/dtb.c \
	$(LIB_SRC_DIR)/heap.c \
	$(LIB_SRC_DIR)/mbox.c \
	$(LIB_SRC_DIR)/mmio.c \
	$(LIB_SRC_DIR)/shell.c \
	$(LIB_SRC_DIR)/string.c \
	$(LIB_SRC_DIR)/uart.c \
	$(LIB_SRC_DIR)/utils.c \

LIB_INCLUDES = \
	-I $(LIB_SRC_DIR)/include \

LIB_BUILD_DIR = $(BUILD_DIR)/lib
LIB_OBJS := $(patsubst %.c,$(LIB_BUILD_DIR)/%.o,$(notdir $(LIB_SRC)))

# Application
APP_SRC_DIR = .

APP_SRC = \
	$(APP_SRC_DIR)/kernel.c \

APP_ASM = \
	$(APP_SRC_DIR)/boot.S \

APP_INCLUDES = \
	-I ./ \
	$(LIB_INCLUDES) \

APP_BUILD_DIR = $(BUILD_DIR)/app
APP_OBJS := $(patsubst %.c,$(APP_BUILD_DIR)/%.o,$(notdir $(APP_SRC)))
APP_OBJS += $(patsubst %.S,$(APP_BUILD_DIR)/%.o,$(notdir $(APP_ASM)))

VPATH += \
	$(LIB_SRC_DIR) \
	$(APP_SRC_DIR) \

# ARM toolchain
CROSS = aarch64-unknown-linux-gnu
CC = $(CROSS)-gcc
AS = $(CROSS)-as
LD = $(CROSS)-ld
OC = $(CROSS)-objcopy
OD = $(CROSS)-objdump
SP = $(CROSS)-strip

# Project
OUT_OBJS = \
	$(LIB_OBJS) \
	$(APP_OBJS) \

BUILD_DIRECTORIES = \
	$(LIB_BUILD_DIR) \
	$(APP_BUILD_DIR) \

PROJ = kernel8
OUT_ELF = ./$(PROJ).elf
OUT_IMG = ./$(PROJ).img
LINK_FILE = linker.ld
CPIO_FILE = ../initramfs/initramfs.cpio
DTB_FILE  = ../devicetree/bcm2710-rpi-3-b-plus.dtb

CCFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles
LDFLAGS = -T$(LINK_FILE)

.PHONY: clean directories out_elf
all: directories $(OUT_OBJS) $(OUT_ELF) $(OUT_IMG)
directories: $(BUILD_DIRECTORIES)
out_elf: directories $(OUT_ELF)
out_img: directories $(OUT_IMG)

# Compile Object Files ---------------------------------------------------------
$(APP_BUILD_DIR)/%.o : %.c
	@echo "[APP Objects] : $@ -----------------------------------------"
	@echo "Building: $<"
	$(CC) $(CCFLAGS) $(APP_INCLUDES) -o $@ -c $<
	@echo "Finished Building: $<"

$(APP_BUILD_DIR)/%.o : %.S
	@echo "[APP Objects] : $@ -----------------------------------------"
	@echo "Building: $<"
	$(CC) $(ASMFLAGS) $(APP_INCLUDES) -o $@ -c $<
	@echo "Finished Building: $<"

$(LIB_BUILD_DIR)/%.o : %.c
	@echo "[LIB Objects] : $@ -----------------------------------------"
	@echo "Building: $<"
	$(CC) $(CCFLAGS) $(LIB_INCLUDES) -o $@ -c $<
	@echo "Finished Building: $<"

# Generate ELF -----------------------------------------------------------------
$(OUT_ELF): $(OUT_OBJS)
	@echo '============================================================'
	@echo 'Building target: $@'
	@echo '--------------------------------'
	$(LD) $(LDFLAGS) -o $@ $(OUT_OBJS)
	$(OD) -d $@ > $(BUILD_DIR)/$(PROJ).objdump
	$(SP) $@
	@echo 'Finished building target: $@'
	@echo ' '

# Generate IMG -----------------------------------------------------------------
$(OUT_IMG): $(OUT_ELF)
	@echo '============================================================'
	@echo 'Building target: $@'
	@echo '--------------------------------'
	$(OC) -O binary $(OUT_ELF) $@
	@echo 'Finished building target: $@'
	@echo ' '

$(BUILD_DIRECTORIES):
	mkdir -p $@

# QEMU
qemu: all
	qemu-system-aarch64 -M raspi3b -kernel $(OUT_IMG) -display none -initrd $(CPIO_FILE) -dtb $(DTB_FILE) -serial null -serial stdio

clean:
	rm -rf *.elf
	rm -rf *.img
	rm -rf $(BUILD_DIR)