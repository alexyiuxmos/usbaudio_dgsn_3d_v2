# The TARGET variable determines what target system the application is
# compiled for. It either refers to an XN file in the source directories
# or a valid argument for the --target option when compiling

ifeq ($(CONFIG),AI)
TARGET = XCORE-AI-EXPLORER
else ifeq ($(CONFIG),200)
TARGET = XCORE-200-EXPLORER
endif

# The USED_MODULES variable lists other module used by the application.
USED_MODULES = lib_locks

# The flags passed to xcc when building the application
# You can also set the following to override flags for a particular language:
# XCC_XC_FLAGS, XCC_C_FLAGS, XCC_ASM_FLAGS, XCC_CPP_FLAGS
# If the variable XCC_MAP_FLAGS is set it overrides the flags passed to
# xcc for the final link (mapping) stage.
XCC_FLAGS_200 = -O2
XCC_FLAGS_AI = -O2

# The VERBOSE variable, if set to 1, enables verbose output from the make system.
VERBOSE = 0

#=============================================================================
# The following part of the Makefile includes the common build infrastructure
# for compiling XMOS applications. You should not need to edit below here.

XMOS_MAKE_PATH ?= ../..
include $(XMOS_MAKE_PATH)/xcommon/module_xcommon/build/Makefile.common
