import ugfx, system, term, sys,time, version, easydraw, orientation

orientation.default()

names = [
	"Niek Blankers", "Sebastian Oort", "Bas van Sisseren",
	"Jeroen Domburg", "Christel Sanders", "Markus Bechtold",
	"Tom Clement", "Prof. Herr Lord I.B. Mobach",
	"Thomas Roos", "Anne Jan Brouwer", "Renze Nicolai",
	"Aram Verstegen", "Arnout Engelen", "Alexandre Dulaunoy",
	"Eric Poulsen", "Damien P. George", "Heikki Juva",
	"Teemu Hakala", "Kliment", "Windytan",
	"Purkkaviritys", "Otto Raila", "Jaga",
	"uGFX", "EMF Badge Team", "MicroPython", "Loboris", " ",
	" ", "BADGE.TEAM"]

def action_exit(pushed):
    if (pushed):
        system.home()

def show_names():
	global names
	c = False
	y = 10
	ugfx.clear()
	
	term.header(True, "About")
	print("Developers:")
	for n in range(0, len(names)):
		if (names[n]==" "):
			break
		print(" - "+names[n])
	
	easydraw.msg("Your badge has been made possible by the following people:", "About", True)
	
	for n in range(0, len(names)):
		easydraw.msg(names[n])
		time.sleep(0.5)
	
	easydraw.msg("Press back or start to quit!")

def main():
    ugfx.input_init()
    ugfx.input_attach(ugfx.BTN_B, action_exit)
    ugfx.input_attach(ugfx.BTN_START, action_exit)
    show_names()
    sys.stdin.read(1) #Wait for any key
    action_exit(True)
    

main()
