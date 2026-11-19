# Build and run the nexus-am cputest suite on arm-am.
#
# Invoked from the top-level Makefile:
#   make cputest              # QEMU mini-virt
#   make cputest-gem5         # gem5 --bare-metal (QEMU-aligned platform)
#
# Each test is a separate bare-metal image, mirroring the upstream layout in
# nexus-am/tests/cputest/tests.

ROOT ?= $(CURDIR)
ARCH ?= arm64-minivirt

# All test images share build/$(ARCH)/common; serialize the sub-makes so that
# "make -j cputest" cannot race on the shared runtime objects.
.NOTPARALLEL:

TESTS := $(sort $(basename $(notdir $(wildcard $(ROOT)/tests/cputest/tests/*.c))))
BUILD_ROOT := $(ROOT)/build/$(ARCH)/cputest

.PHONY: all run run-gem5 clean $(TESTS)

all: $(TESTS)

$(TESTS): %:
	@$(MAKE) -s -C $(ROOT) --no-print-directory \
		ARCH=$(ARCH) \
		APP=cputest/$* \
		APP_NAME=$* \
		APP_DIR=tests/cputest \
		APP_SOURCE=tests/cputest/tests/$*.c \
		BUILD_DIR=$(BUILD_ROOT)/$* \
		test-build

run: all
	@$(ROOT)/tests/cputest/run.sh \
		"$(BUILD_ROOT)" "$(QEMU)" "$(CPU)" "$(MEMORY)"

run-gem5: all
	@GEM5_HOME="$(GEM5_HOME)" $(ROOT)/tests/cputest/run-gem5.sh \
		"$(BUILD_ROOT)" "$(GEM5)" "$(GEM5_CONFIG)" $(GEM5_ARGS)

clean:
	rm -rf $(BUILD_ROOT)
