################################################################################
# Created: 21/09/2018
#
# Original author: Alexy Torres Aurora Dugo
#
# Last modified: 21/09/2018
#
# Last author: Alexy Torres Aurora Dugo
#
# UTK's main makefile
################################################################################

######################### Compile options
.PHONY: all
all: init

init:

ifeq ($(arch), i386)
	@$(MAKE) -f ./Makei386.mk
else ifeq ($(arch), x86_64)
	@$(MAKE) -f ./Makex86_64.mk
else ifeq ($(arch), armv7)
	@$(MAKE) -f ./Makearmv7a15.mk
else
	@echo "\e[1m\e[31mERROR You must select an architecture!\e[22m\e[39m"
	@echo "Example: make arch=[i386, x86_64, armv7]\n"
	@exit 1
endif 

clean:

ifeq ($(arch), i386)
	@$(MAKE) clean -f ./Makei386.mk
else ifeq ($(arch), x86_64)
	@$(MAKE) clean -f ./Makex86_64.mk
else ifeq ($(arch), armv7)
	@$(MAKE) clean -f ./Makearmv7a15.mk
else
	@$(MAKE) clean -f ./Makei386.mk
	@$(MAKE) clean -f ./Makex86_64.mk
	@$(MAKE) clean -f ./Makearmv7a15.mk
endif 

run:
ifeq ($(arch), i386)
	@$(MAKE) run -f ./Makei386.mk
else ifeq ($(arch), x86_64)
	@$(MAKE) run -f ./Makex86_64.mk
else ifeq ($(arch), armv7)
	@$(MAKE) run -f ./Makearmv7a15.mk
else
	@echo "\e[1m\e[31mERROR You must select an architecture!\e[22m\e[39m"
	@echo "Example: make arch=[i386, x86_64, armv7]\n"
	@exit 1
endif 

qemu-test-mode:
ifeq ($(arch), i386)
	@$(MAKE) qemu-test-mode -f ./Makei386.mk
else ifeq ($(arch), x86_64)
	@$(MAKE) qemu-test-mode -f ./Makex86_64.mk
else ifeq ($(arch), armv7)
	@$(MAKE) qemu-test-mode -f ./Makearmv7a15.mk
else
	@echo "\e[1m\e[31mERROR You must select an architecture!\e[22m\e[39m"
	@echo "Example: make arch=[i386, x86_64, armv7]\n"
	@exit 1
endif 

debug:
ifeq ($(arch), i386)
	@$(MAKE) debug -f ./Makei386.mk
else ifeq ($(arch), x86_64)
	@$(MAKE) debug -f ./Makex86_64.mk
else ifeq ($(arch), armv7)
	@$(MAKE) debug -f ./Makearmv7a15.mk
else
	@echo "\e[1m\e[31mERROR You must select an architecture!\e[22m\e[39m"
	@echo "Example: make arch=[i386, x86_64, armv7]\n"
	@exit 1
endif 

######################### Tests
test:
ifeq ($(arch), i386)
	@$(MAKE) test -f ./Makei386.mk
else ifeq ($(arch), x86_64)
	@$(MAKE) test -f ./Makex86_64.mk
else ifeq ($(arch), armv7)
	@$(MAKE) test -f ./Makearmv7a15.mk
else
	@echo "\e[1m\e[31mERROR You must select an architecture!\e[22m\e[39m"
	@echo "Example: make arch=[i386, x86_64, armv7]\n"
	@exit 1
endif 

######################### Image file options
bootable: all
ifeq ($(arch), i386)
	@$(MAKE) bootable -f ./Makei386.mk
else ifeq ($(arch), x86_64)
	@$(MAKE) bootable -f ./Makex86_64.mk
else ifeq ($(arch), armv7)
	@$(MAKE) bootable -f ./Makearmv7a15.mk
else
	@echo "\e[1m\e[31mERROR You must select an architecture!\e[22m\e[39m"
	@echo "Example: make arch=[i386, x86_64, armv7]\n"
	@exit 1
endif 