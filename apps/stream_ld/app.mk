# FOOTPRINT is the address span (K/M/G accepted); PASSES/ITER is the number
# of timed passes; FLUSH is the separate eviction buffer (0 disables).
FOOTPRINT ?= 64M
ITER ?= 1
PASSES ?= $(ITER)
FLUSH ?= 0

APP_CPPFLAGS += -DFOOTPRINT=\"$(FOOTPRINT)\" -DPASSES=$(PASSES) -DFLUSH=\"$(FLUSH)\"
