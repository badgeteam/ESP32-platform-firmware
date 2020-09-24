import display, buttons, system, term, sys,time, version, easydraw, orientation

orientation.default()

names = [
	"Renze Nicolai", "Nikolett",
	"Sebastian Oort", "Kliment",
    "Tom Clement", "Niek Blankers", "Bas van Sisseren",
	"Jeroen Domburg", "Christel Sanders",
	"Markus Bechtold", "Lord I.B. Mobach",
	"Thomas Roos", "Anne Jan Brouwer",
	"Aram Verstegen", "Arnout Engelen",
	"Alexandre Dulaunoy", "Eric Poulsen",
	"Damien P. George", "Heikki Juva",
	"Purkkaviritys", "Peippo",
	"Jaga", "EMF Badge Team",
	"apo", "Walther",
	"MicroPython", "Loboris",
	"ESP32 platform firmware",
	"-  -  -",
	"BADGE.TEAM",
	"github.com/badgeteam",
	"-  -  -",
	"Thank you all!"]

def action_exit(pushed):
	if (pushed):
		system.launcher()

def show_names():
	global names
	c = False
	y = 10
	
	term.header(True, "About")
	for n in range(0, len(names)):
		if (names[n]=="-  -  -"):
			print("")
			continue
		print(names[n], end="   ")
	
	easydraw.msg("Your badge was made possible by the following people:", "ABOUT", True)
	
	for n in range(0, len(names)):
		easydraw.msg(names[n])
		time.sleep(0.8)

def main():
	buttons.attach(buttons.BTN_A, action_exit)
	buttons.attach(buttons.BTN_B, action_exit)
	try:
		buttons.attach(buttons.BTN_START, action_exit)
	except:
		pass
	show_names()
	sys.stdin.read(1) #Wait for any key
	action_exit(True)
	

main()
