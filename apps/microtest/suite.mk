# Build and run the microtest suite.
#
# Every tests/<name>.c is compiled into its own bare-metal image:
#   build/$(ARCH)/microtest/<name>/<name>.{elf,bin}
# so each benchmark is an independent minibench .bin that can be run on QEMU
# or gem5 without sharing state with any other test.
#
# Invoked from the top-level Makefile:
#   make microtest             build and run all images on QEMU
#   make microtest-build       build all images
#   make microtest-gem5        build and run all images on gem5
#   make microtest TEST=ld     only the named image
#
# The test list is discovered from tests/*.c, so adding a benchmark only
# requires dropping a new source file in this directory.

ROOT ?= $(CURDIR)
ARCH ?= arm64-minivirt

# All images share build/$(ARCH)/common; serialize the sub-makes so that a
# parallel top-level make cannot race on the runtime objects.
.NOTPARALLEL:

ALL_TESTS := $(sort $(basename $(notdir $(wildcard $(ROOT)/apps/microtest/tests/*.c))))
ifneq ($(strip $(TEST)),)
ifeq ($(filter $(TEST),$(ALL_TESTS)),)
$(error unknown microtest TEST '$(TEST)'; available: $(ALL_TESTS))
endif
TESTS := $(TEST)
else
TESTS := $(ALL_TESTS)
endif

BUILD_ROOT := $(ROOT)/build/$(ARCH)/microtest

.PHONY: all run run-gem5 $(ALL_TESTS)

all: $(TESTS)

$(TESTS): %:
	@$(MAKE) -s -C $(ROOT) --no-print-directory \
		ARCH=$(ARCH) \
		APP=microtest/$* \
		APP_NAME=$* \
		APP_DIR=apps/microtest \
		APP_SOURCE=apps/microtest/tests/$*.c \
		BUILD_DIR=$(BUILD_ROOT)/$* \
		ITERS="$(ITERS)" \
		build

run: all
	@set -e; for test in $(TESTS); do \
		printf '\n== microtest: %s ==\n' "$$test"; \
		$(MAKE) -s -C $(ROOT) --no-print-directory \
			ARCH=$(ARCH) \
			APP=microtest/$$test \
			APP_NAME=$$test \
			APP_DIR=apps/microtest \
			APP_SOURCE=apps/microtest/tests/$$test.c \
			BUILD_DIR=$(BUILD_ROOT)/$$test \
			ITERS="$(ITERS)" \
			run; \
	done

run-gem5: all
	@set -e; for test in $(TESTS); do \
		printf '\n== microtest: %s ==\n' "$$test"; \
		$(MAKE) -s -C $(ROOT) --no-print-directory \
			ARCH=$(ARCH) \
			APP=microtest/$$test \
			APP_NAME=$$test \
			APP_DIR=apps/microtest \
			APP_SOURCE=apps/microtest/tests/$$test.c \
			BUILD_DIR=$(BUILD_ROOT)/$$test \
			ITERS="$(ITERS)" \
			gem5-run; \
	done
