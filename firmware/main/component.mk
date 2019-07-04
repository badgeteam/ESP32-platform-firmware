#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

COMPONENT_ADD_INCLUDEDIRS := . \
	$(PROJECT_PATH)/components/resource_ssl_letsencrypt \

COMPONENT_EXTRA_INCLUDES := $(PROJECT_PATH)/components/driver_input_mpr121/include
