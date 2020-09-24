import badge, easydraw, woezel, wifi

lvl = badge.nvs_get_u8('ota', 'fixlvl', 0)
if lvl<3:
	if not badge.nvs_get_u16("modaudio", "mixer_ctl_0"):
		badge.nvs_set_u16("modaudio", "mixer_ctl_0", (0 << 15) + (0 << 8) + (1 << 7) + (32 << 0))
		badge.nvs_set_u16("modaudio", "mixer_ctl_1", (0 << 15) + (32 << 8) + (0 << 7) + (32 << 0))
	badge.nvs_set_u8('ota', 'fixlvl', 3)

if lvl<5:
	wifi.connect()
	if wifi.wait(30, True):
		for bloatware in ['event_schedule','de_rode_hack','hacker_gallery','angry_nerds_podcast']:
			# This is ugly, but if 1 app is already up to date we just try to fill the device with the rest
			try:
				easydraw.messageCentered("Updating...\n"+bloatware, True, '/media/flag.png')
				woezel.install(bloatware)
			except:
				pass
		badge.nvs_set_u8('ota', 'fixlvl', 5)
