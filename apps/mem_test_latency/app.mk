# INST is the footprint multiplier used by the upstream test: the flush
# footprint is 256 KiB * INST.  Keep it large enough to evict the target cache
# hierarchy (for a 1 MiB L2 + 6 MiB L3, INST >= 32 is a reasonable start).
INST ?= 16
APP_CPPFLAGS += -DINST=$(INST)
