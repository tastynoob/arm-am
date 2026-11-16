# TEST_TYPE selects the upstream probe: mem (DDR bandwidth) or cache
# (L1/L2 bandwidth sweep).
TEST_TYPE ?= mem
APP_CPPFLAGS += -DTEST_TYPE=$(TEST_TYPE)
