# Test list and source layout for this app.
TESTS := packedtest
TEST_SRC = $(APP_DIR)/main.c

PACKED_ITERS ?= 1000
PACKED_COPIES ?= 12
APP_CPPFLAGS += -DPACKED_ITERS=$(PACKED_ITERS) -DPACKED_COPIES=$(PACKED_COPIES)
