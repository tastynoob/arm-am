# Ported from OpenXiangShan/nexus-am tests/frontendtest/cond_br_test.
TESTS := aliasing_pattern all_patterns alternating always_taken early_exits \
         gradual_transition nested_branches never_taken prime_based_pattern \
         rare_branches switching_pattern three_bit_pattern two_bit_pattern
TEST_SRC = $(APP_DIR)/$(1)/main.c

# Keep the simple C branch patterns visible instead of letting -O2 fold
# always/never-taken conditions away.
APP_CFLAGS += -O0
