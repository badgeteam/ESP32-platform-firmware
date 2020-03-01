# Component Makefile

COMPONENT_SRCDIRS := . fonts png

COMPONENT_ADD_INCLUDEDIRS := png

COMPONENT_EXTRA_INCLUDES := $(PROJECT_PATH)/components/driver_display_hub75/include \
                            $(PROJECT_PATH)/components/driver_display_gxgde0213b1/include \
                            $(PROJECT_PATH)/components/driver_display_erc12864/include \
                            $(PROJECT_PATH)/components/driver_display_ssd1306/include \
                            $(PROJECT_PATH)/components/driver_display_eink/include \
                            $(PROJECT_PATH)/components/driver_display_ili9341/include \
                            $(PROJECT_PATH)/components/driver_display_hub75/include \
                            $(PROJECT_PATH)/components/driver_display_fri3d/include \
                            $(PROJECT_PATH)/components/driver_display_flipdotter/include \
                            $(PROJECT_PATH)/components/driver_display_st7735/include \
                            $(PROJECT_PATH)/components/driver_display_st7789v/include \
                            $(PROJECT_PATH)/components/driver_display_nokia6100/include \
                            $(PROJECT_PATH)/components/driver_display_ledmatrix/include \
                            $(PROJECT_PATH)/components/driver_io_disobey_samd/include \
