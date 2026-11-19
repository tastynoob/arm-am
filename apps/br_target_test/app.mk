# Ported from OpenXiangShan/nexus-am tests/frontendtest/br_target_test.
TESTS := call_branch indirect_branch jump_branch return_branch
TEST_SRC = $(APP_DIR)/$(1)/main.c

# Keep the call/return and indirect-call structure visible.
APP_CFLAGS += -O0
