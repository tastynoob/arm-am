# Test list and source layout for this app.
TESTS := alu chain_alu cmpbr ld st ldp stp ldst ldpstp
TEST_SRC = $(APP_DIR)/$(1)/main.c

# Shared build rules for every apps/throughput/<name> minibench.
#
# The category owns the compilation parameters; the individual sub-minibench
# directories only contain main.c and README.md.

ITERS ?= 100000
APP_CPPFLAGS += -DBENCH_ITERS=$(ITERS)ULL

# Only chain_alu has the independent-chain parameter.
ifeq ($(APP_NAME),chain_alu)
CHAINS ?= 4
APP_CPPFLAGS += -DCHAINS=$(CHAINS)
endif
