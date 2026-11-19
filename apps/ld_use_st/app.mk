# Test list and source layout for this app.
TESTS := ld_use_st
TEST_SRC = $(APP_DIR)/main.c

# ITERS is the loop trip count.
ITERS ?= 100000
APP_CPPFLAGS += -DBENCH_ITERS=$(ITERS)ULL
