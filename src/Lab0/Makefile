BUILD_DIR = ./build

# Application
APP_SRC_DIR = .

APP_ASM = \
	$(APP_SRC_DIR)/a.S \

APP_INCLUDES = \
	-I ./ \

APP_BUILD_DIR = $(BUILD_DIR)/app
APP_OBJS := $(patsubst %.S,$(APP_BUILD_DIR)/%.o,$(notdir $(APP_ASM)))

# ARM toolchain
CROSS = aarch64-none-elf
CC = $(CROSS)-gcc
AS = $(CROSS)-as
LD = $(CROSS)-ld
OD = $(CROSS)-objdump
SP = $(CROSS)-strip

# Project
OUT_OBJS = \
	$(APP_OBJS) \

BUILD_DIRECTORIES = \
	$(APP_BUILD_DIR) \

PROJ = kernel8
OUT_ELF = ./$(PROJ).elf
LINKER_FILE = linker.ld

LDFLAGS = -T$(LINKER_FILE)

.PHONY: clean directories app_compile out_elf
all: directories $(OUT_OBJS) $(OUT_ELF)
directories: $(BUILD_DIRECTORIES)
app_compile: directories $(APP_OBJS)
out_elf: directories $(OUT_ELF)

# Compile Object Files ---------------------------------------------------------
$(APP_BUILD_DIR)/%.o : %.c
	@echo "[APP Objects] : $@ -----------------------------------------"
	@echo "Building: $<"
	$(CC) $(CFLAGS) $(APP_INCLUDES) -o $@ -c $<
	@echo "Finished Building: $<"

$(APP_BUILD_DIR)/%.o : %.S
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

$(BUILD_DIRECTORIES):
	mkdir -p $@

clean:
	rm -rf *.elf
	rm -rf $(BUILD_DIR)