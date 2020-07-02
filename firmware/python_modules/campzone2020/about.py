import system, term

names = [
    "Tom Clement", "Joris Witteman", "Eric van Zandvoort",
    "Alex-Justin de Groot", "Nils", "Roy van Dongen",
    "Jurrien Bloemen", "Steph Janssen", "Stefan Mennes",
    "Sebastian Oort", "Anne Jan Brouwer", "Renze Nicolai",
    "Niek Blankers", "Bas van Sisseren", "Jeroen Domburg",
    "Christel Sanders", "Markus Bechtold",
    "Prof. Herr Lord I.B. Mobach", "Thomas Roos",
    "Aram Verstegen", "Arnout Engelen", "Alexandre Dulaunoy",
    "Eric Poulsen", "Damien P. George", "Heikki Juva",
    "Teemu Hakala", "Kliment", "Windytan",
    "Purkkaviritys", "Otto Raila", "Jaga",
    "EMF Badge Team", "MicroPython", "Loboris",
    "ALLNET China", "Espressif", "Tesorion",
    "BADGE.TEAM"]

def action_exit(pushed):
    if (pushed):
        system.home()

def show_names():
    global names

    term.header(True, "About")
    print("Developers:")
    for n in range(0, len(names)):
        if (names[n]==" "):
            break
        print(" - "+names[n])

def main():
    show_names()

main()
