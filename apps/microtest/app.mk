# Shared settings for every microtest image.
#
# This directory is a multi-image suite: suite.mk supplies APP_SOURCE,
# APP_NAME and BUILD_DIR once per tests/<name>.c, so each benchmark becomes an
# independent .bin.  Invoking the generic app machinery as "make APP=microtest"
# is not the supported interface.
ifeq ($(strip $(APP_SOURCE)),)
$(error apps/microtest is a multi-image suite: use "make microtest" or "make microtest TEST=<name>")
endif

# ITERS overrides the default 100000 loop iterations (e.g. ITERS=10000 on a
# slow gem5 model); suite.mk forwards it to every image.
ITERS ?=
ifneq ($(strip $(ITERS)),)
APP_CPPFLAGS += -DBENCH_ITERS=$(ITERS)ULL
endif
