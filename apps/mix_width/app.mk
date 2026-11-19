# Mixed-instruction throughput: independent instructions from different classes.
TESTS := alu_fp alu_ld alu_st alu_fp_ld_st alu_mul \
         alu_branch_ld_st alu_fp_branch_ld_st alu_mul_branch_ld_st all_types
ITERS ?= 100000
APP_CPPFLAGS += -DBENCH_ITERS=$(ITERS)ULL
TEST_SRC = $(APP_DIR)/$(1)/main.c
