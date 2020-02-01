import ugfx, esp, badge, deepsleep, term, easydraw

def start_app(app, display = True):
	if display:
		easydraw.msg(app, "Loading...", True)
		term.header(True, "Loading application "+app+"...")
	esp.rtcmem_write_string(app)
	deepsleep.reboot()

def home():
	start_app("")

#def start_bpp(duration):
#	term.header(True, "Starting BPP...")
#	print("[BPP] Duration = "+str(duration))
#	esp.rtcmem_write(0,2)
#	esp.rtcmem_write(1,253)
#	deepsleep.reboot()
