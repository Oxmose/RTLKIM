################################################################################
# Created: 21/09/2018
#
# Original author: Alexy Torres Aurora Dugo
#
# Last modified: 21/09/2018
#
# Last author: Alexy Torres Aurora Dugo
#
# RTLK's main makefile
################################################################################

######################### Modules selection

MODULES = Boot Cpu IO Lib Drivers Interrupt BSP Time
ifeq ($(TESTS), TRUE)
MODULES += ../Tests/Tests
endif

######################### Files options
# Kernel name
KERNEL = kernel.bin

# Source directories definition
SRC_DIR    = Src
BUILD_DIR  = Build
INC_DIR    = Includes
CONFIG_DIR = Config
TESTS_DIR  = Tests
BIN_DIR    = Bin

SRC_DIRS = $(MODULES:%=$(SRC_DIR)/%)
OBJ_DIRS = $(MODULES:%=$(BUILD_DIR)/%)

C_SRCS = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
A_SRCS = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.S))
C_OBJS = $(C_SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
A_OBJS = $(A_SRCS:$(SRC_DIR)/%.S=$(BUILD_DIR)/%.o)

# Linker script's name 
LINKER_FILE = Config/linker.ld

######################### Toolchain options
AS = nasm
LD = ld
OBJCOPY = objcopy
QEMU = qemu-system-x86_64

DEBUG_FLAGS = -O0 -g
EXTRA_FLAGS = -O2

CFLAGS = -m32 -gstabs -std=c11 -nostdinc -fno-builtin -nostdlib \
        -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra

ifeq ($(DEBUG), TRUE)
CFLAGS += $(DEBUG_FLAGS) -c
else 
CFLAGS += $(EXTRA_FLAGS) -c
endif

ASFLAGS = -g -f elf -w+gnu-elf-extensions
LDFLAGS = -e loader -T $(LINKER_FILE) -melf_i386
QEMUOPTS = -cpu Nehalem -d guest_errors -rtc base=localtime -m 64 \
           -gdb tcp::1234 -smp 4 -kernel

######################### Compile options
.PHONY: all
all: clean init kernel.bin

init:
	@mkdir -p $(OBJ_DIRS)
	@mkdir -p $(BIN_DIR)

	@echo "C Sources:"
	@echo $(C_SRCS)
	@echo 
	@echo "ASM Sources:"
	@echo $(A_SRCS)
	@echo 

# kernel generation
kernel.bin: $(LINKER_FILE) compile_asm compile_cc
	$(LD) $(LDFLAGS) $(C_OBJS) $(A_OBJS) -o $(BIN_DIR)/$(KERNEL)
	@echo Linked object files
	@echo

compile_asm: $(A_OBJS)
	@echo Compiled assembly sources
	@echo

compile_cc: $(C_OBJS)
	@echo Compiled c sources
	@echo

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
ifeq ($(TESTS), TRUE)
	$(CC) $(CFLAGS) $< -o $@ -I $(INC_DIR) -I $(CONFIG_DIR) -I $(TESTS_DIR)
else 
	$(CC) $(CFLAGS) $< -o $@ -I $(INC_DIR) -I $(CONFIG_DIR)
endif

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.S
	$(AS) $(ASFLAGS) $< -o $@

clean:
	@$(RM) -rf $(BIN_DIR) $(BUILD_DIR) $(TESTS_DIR)/Tests/*.o
	@echo Cleaned build files and binaries
	@echo

######################### Qemu options
run:
	$(QEMU) $(QEMUOPTS) $(BIN_DIR)/$(KERNEL) -serial stdio

qemu-test-mode:
	$(QEMU) $(QEMUOPTS) $(BIN_DIR)/$(KERNEL) -serial mon:stdio -nographic

debug:
	$(QEMU) $(QEMUOPTS) $(BIN_DIR)/$(KERNEL) -S

######################### Tests
test:
	chmod +x launchtests.sh
	./launchtests.sh

######################### Image file options
bootable: all
	cp $(BIN_DIR)/$(KERNEL) ../iso/boot/
	rm -f ../iso/bootable.iso
	grub-mkrescue -o ../iso/bootable.iso ../iso
	VBoxManage startvm "RTLK"
