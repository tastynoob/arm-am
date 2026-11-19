# Ported from OpenXiangShan/nexus-am tests/frontendtest.
TESTS := 2fetch aluwidth brnum brnum2 brnum2_uftb brnum3 brsimple brwidth \
         forloop fpuwidth ifuwidth ittage ras_recursive rastest \
         renamewidth resolve tage1 tage2 tage3 tage4 tage5
TEST_SRC = $(APP_DIR)/$(1)/main.c
