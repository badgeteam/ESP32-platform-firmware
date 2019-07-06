import system, term, sys, time, version, buttons
import defines, rgb

names = [
    "Tom Clement", "Renze Nicolai", "Joris Witteman",
    "Pim de Groot", "Khaled Nassar", "Evan Mandos",
    "Niek Blankers", "Sebastian Oort", "Bas van Sisseren",
    "Jeroen Domburg", "Christel Sanders", "Markus Bechtold",
    "Tom Clement", "Prof. Herr Lord I.B. Mobach",
    "Thomas Roos", "Anne Jan Brouwer",
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

    term.header(True, "About")
    print("Developers:")
    for n in range(0, len(names)):
        if (names[n]==" "):
            break
        print(" - "+names[n])

    rgb.scrolltext("Your badge was made possible by:")
    time.sleep(3)

    for n in range(0, len(names)):
        rgb.scrolltext(names[n])
        time.sleep(0.5)

    rgb.scrolltext("Press A or B to quit!")

def main():
    buttons.register(defines.BTN_A, action_exit)
    buttons.register(defines.BTN_B, action_exit)

    show_names()

main()
