# Test list and source layout for this app.
TESTS := move_elim
TEST_SRC = $(APP_DIR)/main.c

MOVE_ITERS ?= 1000
MOVE_COPIES ?= 16
APP_CPPFLAGS += -DMOVE_ITERS=$(MOVE_ITERS) -DMOVE_COPIES=$(MOVE_COPIES)
