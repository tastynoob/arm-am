# Single-core memory-dependence (RAW/WAR) violation tests.
TESTS := raw_same_addr raw_store_pair raw_train_flip raw_loop_carry raw_loop_late \
         war_same_addr war_same_line mixed
TEST_SRC = $(APP_DIR)/$(1)/main.c
