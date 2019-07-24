import system, term, sys, time, version, buttons
import defines, rgb

names = [
    "Tom Clement", "Renze Nicolai", "Joris Witteman",
    "Pim de Groot", "Khaled Nassar", "Eric van Zandvoort",
    "Niek Blankers", "Sebastian Oort", "Bas van Sisseren",
    "Jeroen Domburg", "Christel Sanders", "Markus Bechtold",
    "Prof. Herr Lord I.B. Mobach", "Thomas Roos", "Anne Jan Brouwer",
    "Aram Verstegen", "Arnout Engelen", "Alexandre Dulaunoy",
    "Eric Poulsen", "Damien P. George", "Heikki Juva",
    "Teemu Hakala", "Kliment", "Windytan",
    "Purkkaviritys", "Otto Raila", "Jaga",
    "EMF Badge Team", "MicroPython", "Loboris",
    "BADGE.TEAM", "CTF{w3lc0metoothehackZONECeeTieEf}"]

def action_exit(pushed):
    if (pushed):
        system.home()

def show_names():
    global names
    c = False
    y = 10

    term.header(True, "About")
    print("Developers:")
    for n in range(0, len(names)):
        if (names[n]==" "):
            break
        print(" - "+names[n])

    rgb.clear()
    rgb.scrolltext("Your badge was made possible by:")
    time.sleep(10)

    for n in range(0, len(names)):
        rgb.clear()
        rgb.scrolltext(names[n])
        time.sleep(5)

    rgb.clear()
    rgb.scrolltext("Press A or B to quit")

def main():
    buttons.register(defines.BTN_A, action_exit)
    buttons.register(defines.BTN_B, action_exit)

    show_names()

main()
