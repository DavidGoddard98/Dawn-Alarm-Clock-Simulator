# "main" pseudo-component makefile

# turn off some compiler warnings
CPPFLAGS += \
  -Wno-all \
  -Wno-error=return-type -Wno-write-strings -Wno-conversion-null \
  -Wno-return-type -Wno-pointer-arith -Wno-cpp -Wno-unused-variable \

# you can include libraries like this:
#9LEDS_LIBS := $(UNPHONE_SRC_DIR) \
#  $(UNPHONE_LIB_DIR)/OneWire \

# no extra libs needed at present
9LEDS_LIBS := 

# include library sources in the build
COMPONENT_SRCDIRS += $(9LEDS_LIBS)
COMPONENT_ADD_INCLUDEDIRS := $(9LEDS_LIBS)
