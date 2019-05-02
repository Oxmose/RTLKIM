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

MODULES  = IO Time Memory Core Lib User Sync Communication Interrupt
INC_ARCH = NONE

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
CONFIG_DIR = Config
TESTS_DIR  = ../Tests
BIN_DIR    = Bin

ifeq ($(arch), i386)
MODULES  += Arch/i386/API Arch/i386/Boot Arch/i386/BSP \
	        Arch/i386/Cpu Arch/i386/Drivers Arch/i386/Memory
INC_ARCH = -I $(INC_DIR)/Arch/i386/
endif 

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
QEMU = qemu-system-i386

DEBUG_FLAGS = -Og -g
EXTRA_FLAGS = -O2

CWARNS = -Waggregate-return -Wcast-align -Wdisabled-optimization \
         -Wdiv-by-zero -Wendif-labels -Wformat-extra-args -Wformat-nonliteral \
		 -Wformat-security -Wformat-y2k -Wimplicit -Wimport -Winit-self \
		 -Winline -Winvalid-pch -Wjump-misses-init -Wlogical-op \
		 -Werror=missing-braces \
		 -Wno-missing-format-attribute -Wmissing-include-dirs -Wmultichar \
		 -Wpointer-arith -Wreturn-type -Wsequence-point -Wsign-compare \
		 -Wstrict-aliasing -Wstrict-aliasing=2 -Wswitch -Wswitch-default \
		 -Werror=undef -Wno-unused -Wvariadic-macros -Wwrite-strings \
		 -Werror=implicit-function-declaration \
		 -Werror=nested-externs -Werror=old-style-definition \
		 -Werror=strict-prototypes -Wabi -Waddress -Waggregate-return \
		 -Waggressive-loop-optimizations -Warray-bounds \
		 -Wattributes -Wbad-function-cast -Wbool-compare \
		 -Wbuiltin-macro-redefined \
		 -Wc99-c11-compat -Wcast-align \
		 -Wcast-qual -Wchar-subscripts -Wchkp \
		 -Wclobbered -Wcomment \
		 -Wconversion -Wcoverage-mismatch \
		 -Wcpp  -Wdate-time -Wdeclaration-after-statement \
		 -Wdeprecated \
		 -Wdeprecated-declarations -Wdesignated-init -Wdisabled-optimization \
		 -Wdiscarded-array-qualifiers -Wdiscarded-qualifiers -Wdiv-by-zero \
		 -Wdouble-promotion -Wempty-body -Wendif-labels \
		 -Wenum-compare \
		 -Wextra -Wfloat-equal -Wformat-contains-nul -Wformat-extra-args \
		 -Wformat-nonliteral -Wformat-security -Wformat-signedness \
		 -Wformat-y2k \
		 -Wformat-zero-length -Wfree-nonheap-object \
		 -Wignored-qualifiers -Wimplicit -Wimplicit-function-declaration \
		 -Wimplicit-int \
		 -Wincompatible-pointer-types -Winit-self \
		 -Winline -Wint-conversion -Wint-to-pointer-cast \
		 -Winvalid-memory-model \
		 -Winvalid-pch -Wjump-misses-init \
		 -Wlogical-not-parentheses -Wlogical-op \
		 -Wmain -Wmaybe-uninitialized -Wmemset-transposed-args \
		 -Wmissing-braces -Wmissing-declarations -Wmissing-field-initializers \
		 -Wmissing-include-dirs -Wmissing-parameter-type -Wmissing-prototypes \
		 -Wmultichar -Wnarrowing -Wnested-externs \
		 -Wnonnull -Wodr \
		 -Wold-style-declaration -Wold-style-definition \
		 -Wopenmp-simd -Woverflow -Woverlength-strings \
		 -Woverride-init -Wpacked -Wpacked-bitfield-compat -Wpadded \
		 -Wparentheses -Wpedantic -Wpointer-arith \
		 -Wpointer-sign -Wpointer-to-int-cast -Wpragmas \
		 -Wredundant-decls -Wreturn-local-addr \
		 -Wreturn-type -Wsequence-point -Wshadow \
		 -Wshift-count-negative -Wshift-count-overflow -Wsign-compare \
		 -Wsizeof-array-argument \
		 -Wsizeof-pointer-memaccess -Wstack-protector \
		 -Wstrict-prototypes -Wsuggest-attribute=const \
		 -Wsuggest-attribute=format -Wsuggest-attribute=noreturn \
		 -Wsuggest-attribute=pure -Wsuggest-final-methods \
		 -Wsuggest-final-types -Wswitch \
		 -Wswitch-bool -Wswitch-default -Wswitch-enum -Wsync-nand \
		 -Wsystem-headers -Wtraditional \
		 -Wtraditional-conversion -Wtrampolines -Wtrigraphs -Wtype-limits \
		 -Wundef -Wuninitialized \
		 -Wunknown-pragmas -Wunsafe-loop-optimizations \
		 -Wunsuffixed-float-constants -Wunused -Wunused-but-set-parameter \
		 -Wunused-but-set-variable \
		 -Wunused-function -Wunused-label -Wunused-local-typedefs \
		 -Wunused-macros -Wunused-parameter -Wunused-result \
		 -Wunused-value -Wunused-variable \
		 -Wvarargs -Wvariadic-macros \
		 -Wvector-operation-performance -Wvla \
		 -Wvolatile-register-var -Wwrite-strings

CFLAGS = -m32 -std=c11 -nostdinc -fno-builtin -nostdlib -fno-stack-protector \
         -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c -fno-pie \
		 -no-pie -MD -ffreestanding

ifeq ($(WARNS), TRUE)
CFLAGS += $(CWARNS)
endif

ifeq ($(DEBUG), TRUE)
CFLAGS += $(DEBUG_FLAGS)
else
CFLAGS += $(EXTRA_FLAGS)
endif

ASFLAGS = -g -f elf -w+gnu-elf-extensions
LDFLAGS = -e loader -T $(LINKER_FILE) -melf_i386 -no-pie
QEMUOPTS = -cpu Nehalem -d guest_errors -rtc base=localtime -m 64M \
           -gdb tcp::1234 -smp 4 \
		   -drive format=raw,file=../Peripherals/hdd_primary_master.img -kernel

######################### Compile options
.PHONY: all
all: init kernel.bin

init:

ifeq ($(arch), i386)
	@echo "\e[1m\e[34mI386 architecture selected\e[22m\e[39m"
else
	@echo "\e[1m\e[31mERROR You must select an architecture!\e[22m\e[39m"
	@echo "Example: make arch=[i386, armv7]\n"
	@exit 1
endif 

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
	$(CC) $(CFLAGS) $< -o $@ -I $(INC_DIR) $(INC_ARCH) -I $(CONFIG_DIR) -I $(TESTS_DIR)
else
	@echo  "\e[32m$< \e[22m\e[39m-> \e[1m\e[94m$@\e[22m\e[39m"
	$(CC) $(CFLAGS) $< -o $@ -I $(INC_DIR) $(INC_ARCH) -I $(CONFIG_DIR)
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
	VBoxManage startvm "RTLK"
