import badge, easydraw, version

lvl = badge.nvs_get_u8('ota', 'fixlvl', 0)

if lvl<4:
	badge.nvs_set_u8('ota', 'fixlvl', 4)
