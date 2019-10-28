# "main" pseudo-component makefile

# add in the libraries
PR_UP_THING_LIBS := \
  ../lib/ESPAsyncWebServer/src \
  ../lib/AsyncTCP/src \
  ../lib/WiFiManager \

COMPONENT_SRCDIRS += $(PR_UP_THING_LIBS)
COMPONENT_ADD_INCLUDEDIRS := $(PR_UP_THING_LIBS)
