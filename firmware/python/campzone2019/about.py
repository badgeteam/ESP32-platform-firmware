import ugfx, appglue, term, sys,time, version, easydraw

names = [
	"Niek Blankers", "Sebastian Oort", "Bas van Sisseren",
	"Jeroen Domburg", "Christel Sanders", "Markus Bechtold",
	"Thomas Roos", "Anne Jan Brouwer", "Renze Nicolai",
	"Aram Verstegen", "Arnout Engelen", "Alexandre Dulaunoy",
	"Eric Poulsen", "Damien P. George", "Heikki Juva",
	"Teemu Hakala", "Kliment", "Windytan",
	"Purkkaviritys", "Otto Raila", "Jaga",
	"uGFX", "EMF Badge Team", "Tom Clement", "Khaled Nassar", "Evan Mandos", "",
	"", "Press back or start", "to quit!"]

def action_exit(pushed):
    if (pushed):
        appglue.home()

def show_names():
	global names
	c = False
	y = 10
	nos = 0
	ugfx.clear()
	
	term.header(True, "About")
	print("Developers:")
	for n in range(0, len(names)):
		if (names[n]==""):
			break
		print(" - "+names[n])
	
	nos = 9999
	
	for n in range(0, len(names)):
		if (nos > 3):
			if (nos < 900):
				time.sleep(2)
			nos = 0
			easydraw.msg("", "Thank you!", True)
		easydraw.msg(names[n])
		nos += 1

def main():
    ugfx.input_init()
    ugfx.input_attach(ugfx.BTN_B, action_exit)
    ugfx.input_attach(ugfx.BTN_START, action_exit)
    show_names()
    sys.stdin.read(1) #Wait for any key
    action_exit(True)
    

main()
