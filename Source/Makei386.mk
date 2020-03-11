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

MODULES  = IO Time Memory Core Lib User Sync Communication Interrupt \
		   Arch/i386/API Arch/i386/Boot Arch/i386/BSP \
	       Arch/i386/Cpu Arch/i386/Drivers Arch/i386/Memory

ifeq ($(TESTS), TRUE)
MODULES += ../../Tests/Tests
endif

######################### Files options
# Kernel name
KERNEL = kernel.bin

# Source directories definition
SRC_DIR    = Src
BUILD_DIR  = Build
INC_DIR    = Includes
INC_ARCH   = $(INC_DIR)/Arch/i386/
CONFIG_DIR = Config/i386
TESTS_DIR  = ../Tests
BIN_DIR    = Bin

SRC_DIRS = $(MODULES:%=$(SRC_DIR)/%)
OBJ_DIRS = $(MODULES:%=$(BUILD_DIR)/%)

C_SRCS = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
A_SRCS = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.S))
C_OBJS = $(C_SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
A_OBJS = $(A_SRCS:$(SRC_DIR)/%.S=$(BUILD_DIR)/%.o)

# Linker script's name
LINKER_FILE = $(CONFIG_DIR)/linker.ld

######################### Toolchain options
AS = nasm
LD = ld
OBJCOPY = objcopy
QEMU = qemu-system-i386

DEBUG_FLAGS = -O0 -g
EXTRA_FLAGS = -O2

CFLAGS = -m32 -std=c11 -nostdinc -fno-builtin -nostdlib -fno-stack-protector \
         -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c -fno-pie \
		 -no-pie -MD -ffreestanding

ifeq ($(DEBUG), TRUE)
CFLAGS += $(DEBUG_FLAGS)
else
CFLAGS += $(EXTRA_FLAGS)
endif

ASFLAGS = -g -f elf -w+gnu-elf-extensions
LDFLAGS = -e loader -T $(LINKER_FILE) -melf_i386 -no-pie
QEMUOPTS = -cpu Nehalem -d guest_errors -rtc base=localtime -m 256M \
           -gdb tcp::1234 -smp 4 \
		   -drive format=raw,file=../Peripherals/hdd_primary_master.img -kernel

######################### Compile options
.PHONY: all
all: init kernel.bin

init:
	@echo "\e[1m\e[34mI386 architecture selected\e[22m\e[39m"
	@mkdir -p $(OBJ_DIRS)
	@mkdir -p $(BIN_DIR)

# kernel generation
kernel.bin: $(LINKER_FILE) compile_asm compile_cc
	@echo  "\e[32mCreating Binary file \e[22m\e[39m-> \e[1m\e[94m$@\e[22m\e[39m"
	@$(LD) $(LDFLAGS) $(C_OBJS) $(A_OBJS) -o $(BIN_DIR)/$(KERNEL)
	@echo "\e[1m\e[94m============ Link finished ============\e[22m\e[39m"
	@echo

compile_asm: $(A_OBJS)
	@echo "\e[1m\e[94m============ Compiled ASM sources ============\e[22m\e[39m"
	@echo

compile_cc: $(C_OBJS)
	@echo "\e[1m\e[94m============ Compiled C sources ============\e[22m\e[39m"
	@echo

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
ifeq ($(DEBUG), TRUE)
	@echo -n "DEBUG "
endif
ifeq ($(TESTS), TRUE)
	@echo  "\e[32m$< \e[22m\e[39m-> \e[1m\e[94m$@\e[22m\e[39m"
	$(CC) $(CFLAGS) $< -o $@ -I $(INC_DIR) -I $(INC_ARCH) -I $(CONFIG_DIR) -I $(TESTS_DIR)
else
	@echo  "\e[32m$< \e[22m\e[39m-> \e[1m\e[94m$@\e[22m\e[39m"
	$(CC) $(CFLAGS) $< -o $@ -I $(INC_DIR) -I $(INC_ARCH) -I $(CONFIG_DIR)
endif

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.S
	@echo  "\e[32m$< \e[22m\e[39m-> \e[1m\e[94m$@\e[22m\e[39m"
	@$(AS) $(ASFLAGS) $< -o $@

clean:
	@$(RM) -rf $(BIN_DIR) $(BUILD_DIR)
	@$(RM) -f $(TESTS_DIR)/Tests/*.o $(TESTS_DIR)/Tests/*.d
	@$(RM) -f ../Image/boot/$(KERNEL)

	@echo "\e[1m\e[94m============ Cleaning Object files and Binaries ============\e[22m\e[39m"
	@echo


# Check header files modifications
-include $(C_OBJS:.o=.d)
-include $(A_OBJS:.o=.d)

######################### Qemu options
run:
	@echo "\e[1m\e[94m============ Running on Qemu ============\e[22m\e[39m"
	@$(QEMU) $(QEMUOPTS) $(BIN_DIR)/$(KERNEL) -serial stdio

qemu-test-mode:
	@echo "\e[1m\e[94m============ Running on Qemu TEST MODE ============\e[22m\e[39m"
	@$(QEMU) $(QEMUOPTS) $(BIN_DIR)/$(KERNEL) -serial mon:stdio -nographic

debug:
	@echo "\e[1m\e[94m============ Running on Qemu DEBUG MODE ============\e[22m\e[39m"
	@$(QEMU) $(QEMUOPTS) $(BIN_DIR)/$(KERNEL) -S

######################### Tests
test:
	chmod +x launchtests.sh
	./launchtests.sh

######################### Image file options
bootable: all
	@echo "\e[1m\e[94m============ Creating Bootable ISO ============\e[22m\e[39m"
	cp $(BIN_DIR)/$(KERNEL) ../Image/boot/
	@$(RM) -f ../Image/bootable.iso
	grub-mkrescue -o ../Image/bootable.iso ../Image
	@$(RM) -f ../Image/boot/$(KERNEL)
