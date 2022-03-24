APP_SRC_DIR = .

APP_ASM = \
	$(APP_SRC_DIR)/app.s \

APP_OBJS := $(patsubst %.s,%.o,$(notdir $(APP_ASM)))

# ARM toolchain
CROSS = aarch64-unknown-linux-gnu
CC = $(CROSS)-gcc
AS = $(CROSS)-as
LD = $(CROSS)-ld
OC = $(CROSS)-objcopy
OD = $(CROSS)-objdump
SP = $(CROSS)-strip

# Project
PROJ = app

OUT_OBJS = $(APP_OBJS)
OUT_ELF = ./$(PROJ).elf
OUT_IMG = ./$(PROJ).img

.PHONY:  all clean
all: cpio

%.o : %.s
	@echo "[APP Objects] : $@ -----------------------------------------"
	@echo "Building: $<"
	$(CC) $(ASMFLAGS) $(APP_INCLUDES) -o $@ -c $<
	@echo "Finished Building: $<"

# Generate ELF -----------------------------------------------------------------
$(OUT_ELF): $(OUT_OBJS)
	@echo '============================================================'
	@echo 'Building target: $@'
	@echo '--------------------------------'
	$(LD) $(LDFLAGS) -o $@ $(OUT_OBJS)
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

cpio: $(OUT_IMG)
	cp $(OUT_IMG) ./rootfs/$(OUT_IMG)
	rm -rf *.o
	rm -rf *.elf
	rm -rf *.img
	cd ./rootfs; find . | cpio -o -H newc > ../initramfs.cpio

clean:
	rm -rf *.o
	rm -rf *.elf
	rm -rf *.img
	rm -rf *.cpio