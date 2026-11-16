# Shared build settings for every cputest program.
#
# The nexus-am cputest suite relies on two's complement wraparound (e.g.
# add.c drives signed int overflow); -fwrapv keeps that behaviour well-defined
# at the -O2 used by the rest of this repo.
APP_CFLAGS += -fwrapv
