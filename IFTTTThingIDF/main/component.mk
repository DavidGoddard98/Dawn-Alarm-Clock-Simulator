# "main" pseudo-component makefile

UNPHONE_DIR=../../../../../unphone
UNPHONE_LIB_DIR=$(UNPHONE_DIR)/lib
#UNPHONE_SRC_DIR=$(UNPHONE_DIR)/src

# turn off some compiler warnings
CPPFLAGS += \
  -Wno-all \
  -Wno-error=return-type -Wno-write-strings -Wno-conversion-null \
  -Wno-return-type -Wno-pointer-arith -Wno-cpp -Wno-unused-variable \

# extra libs
IFTTT_LIBS := $(UNPHONE_SRC_DIR) \
  $(UNPHONE_LIB_DIR)/movingAvg/src \

# include library sources in the build
COMPONENT_SRCDIRS += $(IFTTT_LIBS)
COMPONENT_ADD_INCLUDEDIRS := $(IFTTT_LIBS)
