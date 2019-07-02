#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

COMPONENT_ADD_INCLUDEDIRS := .

COMPONENT_EXTRA_INCLUDES := \
	$(PROJECT_PATH)/components/driver_bus_i2c/include \
	$(PROJECT_PATH)/components/driver_bus_vspi/include \
