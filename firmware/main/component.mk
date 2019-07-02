#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

COMPONENT_ADD_INCLUDEDIRS := .

COMPONENT_EXTRA_INCLUDES := $(PROJECT_PATH)/components/driver-bus-i2c/include
COMPONENT_EXTRA_INCLUDES += $(PROJECT_PATH)/components/driver-leds-hub75/include
