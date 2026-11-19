# Test list and source layout for this app.
TESTS := chain_ld
TEST_SRC = $(APP_DIR)/main.c

# FOOTPRINT is the address span (K/M/G accepted); PASSES/ITER is the number
# of timed passes; FLUSH is the separate eviction buffer (0 disables).
FOOTPRINT ?= 1M
ITER ?= 1
PASSES ?= $(ITER)
FLUSH ?= 64M

APP_CPPFLAGS += -DFOOTPRINT=\"$(FOOTPRINT)\" -DPASSES=$(PASSES) -DFLUSH=\"$(FLUSH)\"
