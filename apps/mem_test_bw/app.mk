# Test list and source layout for this app.
TESTS := mem_test_bw
TEST_SRC = $(APP_DIR)/main.cpp

# TEST_TYPE selects the upstream probe: mem (DDR bandwidth) or cache
# (L1/L2 bandwidth sweep).
TEST_TYPE ?= mem
APP_CPPFLAGS += -DTEST_TYPE=$(TEST_TYPE)
