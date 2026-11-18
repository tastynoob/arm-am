ARCH ?= arm64-minivirt
APP ?= minibench
APP_NAME ?= $(notdir $(APP))
APP_DIR ?= apps/$(APP)

# Local machine paths (QEMU, toolchain, gem5).  Copy config.local.mk.example
# and adjust it, or override the variables on the make command line.
-include config.local.mk

TOOLCHAIN_DIR ?= $(wildcard $(HOME)/tools/arm-gnu-toolchain-13.2.Rel1)
CROSS_COMPILE ?= $(if $(TOOLCHAIN_DIR),$(TOOLCHAIN_DIR)/bin/,)aarch64-none-elf-
QEMU ?= qemu-system-aarch64
CPU ?= neoverse-n2
MEMORY ?= 8G
UART_BASE ?= 0x13000000

# The gem5 tree must contain the --bare-metal QEMU-aligned platform change
# (see gem5/README.md and gem5/patch_baremetal_qemu_aligned.py).
#
# The current bare-metal interface takes the flat .bin image through
# --generic-arm-cpt --raw-cpt (not --kernel <elf>), and can cross-check it
# against QEMU with --enable-difftest.
GEM5_HOME ?= $(HOME)/workspace/GEM5-kmhv3-aarch64
GEM5 ?= $(GEM5_HOME)/build/ARM/gem5.fast
GEM5_CONFIG ?= configs/example/idealkmhv3_arm.py
GEM5_OUTDIR ?= $(BUILD_DIR)/m5out-gem5
DIFFTEST_REF_SO ?= $(HOME)/workspace/qemu/build/libqemu-aarch64-difftest.so
GEM5_DIFFTEST ?= $(if $(wildcard $(DIFFTEST_REF_SO)),--enable-difftest --difftest-ref-so $(DIFFTEST_REF_SO))
GEM5_ARGS ?=

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump

# Per-app overrides: APP_SOURCE, APP_NAME, APP_CPPFLAGS, APP_CFLAGS.
-include $(APP_DIR)/app.mk

APP_SOURCE ?= $(firstword $(wildcard $(APP_DIR)/main.c $(APP_DIR)/main.cpp))
ifeq ($(strip $(APP_SOURCE)),)
$(error No source found: expected $(APP_DIR)/main.c or $(APP_DIR)/main.cpp)
endif

COMMON_BUILD ?= build/$(ARCH)/common
BUILD_DIR ?= build/$(ARCH)/$(APP_NAME)

ELF := $(BUILD_DIR)/$(APP_NAME).elf
BIN := $(BUILD_DIR)/$(APP_NAME).bin

BASE_CPPFLAGS := -Iam/include
BASE_CFLAGS := -march=armv9-a -ffreestanding -fno-pic -fno-pie \
	-fno-stack-protector -ffunction-sections -fdata-sections -O2 \
	-Wall -Wextra -MMD -MP
BASE_ASFLAGS := -march=armv9-a -ffreestanding -fno-pic -fno-pie \
	-fno-stack-protector -MMD -MP
LDFLAGS := -nostdlib -static -no-pie \
	-Wl,-T,am/src/platform/mini-virt/linker.ld -Wl,--gc-sections \
	-Wl,--build-id=none -Wl,--no-warn-rwx-segments

CPPFLAGS := $(BASE_CPPFLAGS) -I$(APP_DIR) -I$(APP_DIR)/include $(APP_CPPFLAGS)
CFLAGS := $(BASE_CFLAGS) $(APP_CFLAGS)
CXXFLAGS := $(CFLAGS) -std=c++17 -fno-exceptions -fno-rtti \
	-fno-threadsafe-statics
ASFLAGS := $(BASE_ASFLAGS)

RUNTIME_OBJECTS := $(COMMON_BUILD)/start.o $(COMMON_BUILD)/newlib_syscalls.o \
	$(COMMON_BUILD)/platform.o $(COMMON_BUILD)/am.o \
	$(COMMON_BUILD)/ioe.o $(COMMON_BUILD)/klib.o \
	$(COMMON_BUILD)/libc_shim.o
APP_OBJECT := $(BUILD_DIR)/main.o
DEPFILES := $(RUNTIME_OBJECTS:.o=.d) $(APP_OBJECT:.o=.d)

# Rebuild objects when compile flags (e.g. TEST_TYPE/INST/UART_BASE) change.
# The stamp file is only touched when its contents actually differ.
COMMON_STAMP := $(COMMON_BUILD)/.build-config
APP_STAMP := $(BUILD_DIR)/.build-config

MICROTEST_TESTS := $(sort $(basename $(notdir $(wildcard apps/microtest/tests/*.c))))

.PHONY: all build run gem5-run clean list help cputest cputest-build \
	cputest-gem5 microtest microtest-build microtest-gem5 FORCE

all: $(BIN)

build: all

list:
	@printf 'apps:   %s\n' '$(notdir $(wildcard apps/*))'
	@printf 'microtest: %s\n' '$(MICROTEST_TESTS)'
	@printf 'suites: cputest\n'

help:
	@printf '%s\n' \
		'make APP=<app> build       build one app' \
		'make APP=<app> run         build and run one app under QEMU' \
		'make APP=<app> gem5-run    run the raw .bin on gem5 (--bare-metal)' \
		'make microtest             build and run every microtest .bin on QEMU' \
		'make microtest-build       build every microtest .bin (one per test)' \
		'make microtest-gem5        run every microtest .bin on gem5' \
		'make microtest TEST=ld     just one microtest .bin' \
		'make cputest               build and run tests/cputest on QEMU' \
		'make cputest-build         build tests/cputest only' \
		'make cputest-gem5          run tests/cputest on gem5' \
		'make APP=mem_test_latency INST=16 run' \
		'make APP=mem_test_bw TEST_TYPE=cache run' \
		'make clean                 remove build/' \
		'' \
		'Overridable: ARCH APP QEMU CPU MEMORY UART_BASE GEM5_HOME GEM5 GEM5_CONFIG GEM5_OUTDIR DIFFTEST_REF_SO GEM5_ARGS ITERS TEST TOOLCHAIN_DIR CROSS_COMPILE'

$(COMMON_BUILD) $(BUILD_DIR):
	@mkdir -p $@

FORCE:

$(COMMON_STAMP): FORCE | $(COMMON_BUILD)
	@printf '%s\n' '$(BASE_CPPFLAGS)' '$(BASE_CFLAGS)' '$(BASE_ASFLAGS)' \
		'UART_BASE=$(UART_BASE)' > $@.tmp
	@if cmp -s $@.tmp $@; then rm -f $@.tmp; else mv $@.tmp $@; fi

$(APP_STAMP): FORCE | $(BUILD_DIR)
	@printf '%s\n' '$(APP_SOURCE)' '$(CPPFLAGS)' '$(CFLAGS)' \
		'$(CXXFLAGS)' > $@.tmp
	@if cmp -s $@.tmp $@; then rm -f $@.tmp; else mv $@.tmp $@; fi

$(RUNTIME_OBJECTS): $(COMMON_STAMP)

$(COMMON_BUILD)/start.o: am/src/arm64/start.S | $(COMMON_BUILD)
	$(CC) $(ASFLAGS) -x assembler-with-cpp -c $< -o $@

$(COMMON_BUILD)/newlib_syscalls.o: am/src/arm64/newlib_syscalls.c | $(COMMON_BUILD)
	$(CC) $(BASE_CPPFLAGS) $(BASE_CFLAGS) -mcmodel=large -c $< -o $@

$(COMMON_BUILD)/platform.o: am/src/platform/mini-virt/platform.c | $(COMMON_BUILD)
	$(CC) $(BASE_CPPFLAGS) $(BASE_CFLAGS) -DUART_BASE=$(UART_BASE)UL -c $< -o $@

$(COMMON_BUILD)/libc_shim.o: am/src/arm64/libc_shim.c | $(COMMON_BUILD)
	$(CC) $(BASE_CPPFLAGS) $(BASE_CFLAGS) -march=armv8-a \
		-mgeneral-regs-only -fno-builtin -c $< -o $@

$(COMMON_BUILD)/am.o: am/src/arm64/am.c | $(COMMON_BUILD)
	$(CC) $(BASE_CPPFLAGS) $(BASE_CFLAGS) -mcmodel=large -c $< -o $@

$(COMMON_BUILD)/ioe.o: am/src/arm64/ioe.c | $(COMMON_BUILD)
	$(CC) $(BASE_CPPFLAGS) $(BASE_CFLAGS) -c $< -o $@

$(COMMON_BUILD)/klib.o: am/src/arm64/klib.c | $(COMMON_BUILD)
	$(CC) $(BASE_CPPFLAGS) $(BASE_CFLAGS) -c $< -o $@

$(APP_OBJECT): $(APP_SOURCE) $(APP_STAMP) | $(BUILD_DIR)
ifeq ($(suffix $(APP_SOURCE)),.cpp)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
else
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
endif

$(ELF): $(RUNTIME_OBJECTS) $(APP_OBJECT) am/src/platform/mini-virt/linker.ld
	$(CC) $(LDFLAGS) -o $@ $(RUNTIME_OBJECTS) $(APP_OBJECT) \
		-Wl,--start-group -lc -lm -lgcc -Wl,--end-group

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

run: $(BIN)
	$(QEMU) -M mini-virt,platform=bm -cpu $(CPU) -smp 1 -m $(MEMORY) \
		-nographic -monitor none -kernel $(BIN)

# gem5 --bare-metal: RAM 0x40000000, SimpleUart 0x13000000, HLT #0.
# The bare-metal interface loads the flat .bin image, not the ELF.
gem5-run: $(BIN)
	cd $(GEM5_HOME) && $(GEM5) -d $(abspath $(GEM5_OUTDIR)) \
		$(GEM5_CONFIG) --raw-cpt --bare-metal \
		--generic-arm-cpt $(abspath $(BIN)) $(GEM5_DIFFTEST) $(GEM5_ARGS)

microtest:
	@$(MAKE) --no-print-directory -f apps/microtest/suite.mk \
		ROOT=$(CURDIR) ARCH=$(ARCH) TEST="$(TEST)" ITERS="$(ITERS)" \
		QEMU="$(QEMU)" CPU=$(CPU) MEMORY=$(MEMORY) all run

microtest-build:
	@$(MAKE) --no-print-directory -f apps/microtest/suite.mk \
		ROOT=$(CURDIR) ARCH=$(ARCH) TEST="$(TEST)" ITERS="$(ITERS)" all

microtest-gem5:
	@$(MAKE) --no-print-directory -f apps/microtest/suite.mk \
		ROOT=$(CURDIR) ARCH=$(ARCH) TEST="$(TEST)" ITERS="$(ITERS)" \
		GEM5_HOME="$(GEM5_HOME)" GEM5="$(GEM5)" \
		GEM5_CONFIG="$(GEM5_CONFIG)" DIFFTEST_REF_SO="$(DIFFTEST_REF_SO)" \
		GEM5_ARGS="$(GEM5_ARGS)" all run-gem5

cputest:
	@$(MAKE) --no-print-directory -f tests/cputest/suite.mk \
		ROOT=$(CURDIR) ARCH=$(ARCH) QEMU="$(QEMU)" CPU=$(CPU) \
		MEMORY=$(MEMORY) all run

cputest-build:
	@$(MAKE) --no-print-directory -f tests/cputest/suite.mk \
		ROOT=$(CURDIR) ARCH=$(ARCH) all

cputest-gem5:
	@$(MAKE) --no-print-directory -f tests/cputest/suite.mk \
		ROOT=$(CURDIR) ARCH=$(ARCH) GEM5="$(GEM5)" \
		GEM5_HOME="$(GEM5_HOME)" GEM5_CONFIG="$(GEM5_CONFIG)" \
		GEM5_ARGS="$(GEM5_ARGS)" all run-gem5

clean:
	rm -rf build

-include $(DEPFILES)
