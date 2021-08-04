set(srcs
	"main.cpp"
)

set(includes
	""
)

set(requires
	"driver_buses"
	"driver_display_st7789v"
	"driver_framebuffer"
	"python"
)

idf_component_register(
	SRCS "${srcs}"
	INCLUDE_DIRS ${includes}
	REQUIRES ${requires}
)
