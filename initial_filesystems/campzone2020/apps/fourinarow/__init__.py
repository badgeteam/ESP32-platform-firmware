import display, keypad, time

on = 0xFF00FF # purple
off = 0x000000 # uit
player1 = 0x00FF00 # green
player2 = 0xFF0000 # red
playingfield = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0] # playing field 
game_over = 0 # game has finished
current_player = 1 # 1 = player1, 2 = player2, 0 is not a player/placeholder
winnaar = 0

# clear playing field
display.drawFill(0x050505)
display.flush()


# clean display
def cleandisplay():
    display.drawFill(0x050505)
    display.flush()

# draw the playing board
def drawplayingfield():
    for z in range(0, 16):
        x, y = z % 4, int(z / 4)
        if playingfield[z] == 1:
            color = player1
        elif playingfield[z] == 2:
            color = player2
        else:
            color = off
        display.drawPixel(x, y, color)
    display.flush()


# check for horizontal and vertical 4 in a row
def checkplayingfield():
    global winnaar
    global game_over
    for x in range(1,3):
        # first horizontal row 
        if (playingfield[0] == x and playingfield[1] == x and playingfield[2] == x and playingfield[3] == x):
            print("Found a winner {0}".format(x))
            winnaar = x
        if (playingfield[4] == x and playingfield[5] == x and playingfield[6] == x and playingfield[7] == x):
            print("Found a winner {0}".format(x))
            winnaar = x
        if (playingfield[8] == x and playingfield[9] == x and playingfield[10] == x and playingfield[11] == x):
            print("Found a winner {0}".format(x))
            winnaar = x
        if (playingfield[12] == x and playingfield[13] == x and playingfield[14] == x and playingfield[15] == x):
            print("Found a winner {0}".format(x))
            winnaar = x
        # second vertical colums
        if (playingfield[0] == x and playingfield[4] == x and playingfield[8] == x and playingfield[12] == x):
            print("Found a winner {0}".format(x))
            winnaar = x
        if (playingfield[1] == x and playingfield[5] == x and playingfield[9] == x and playingfield[13] == x):
            print("Found a winner {0}".format(x))
            winnaar = x
        if (playingfield[2] == x and playingfield[6] == x and playingfield[10] == x and playingfield[14] == x):
            print("Found a winner {0}".format(x))
            winnaar = x
        if (playingfield[3] == x and playingfield[7] == x and playingfield[11] == x and playingfield[15] == x):
            print("Found a winner {0}".format(x))
            winnaar = x
        # diagnal
        if (playingfield[0] == x and playingfield[5] == x and playingfield[10] == x and playingfield[15] == x):
            print("Found a winner {0}".format(x))
            winnaar = x
        if (playingfield[3] == x and playingfield[6] == x and playingfield[9] == x and playingfield[12] == x):
            print("Found a winner {0}".format(x))
            winnaar = x

        if winnaar == 1 or winnaar == 2:
            game_over = 1

    count = 0
    for z in range(0, 16):
        if playingfield[z] != 0:
            count += 1
    if count == 16:
        game_over = 1
        winnaar = 0


def drawwinningfield():
    global winnaar
    global game_over
    global currentplayer
    for a in range(0,5):
        if winnaar == 1:
            color = player1
        elif winnaar == 2:
            color = player2
        else:
            color = on
        display.drawFill(color)
        display.flush()
        time.sleep(1)
    
        color = off
        display.drawFill(color)
        display.flush()
        time.sleep(1)
    game_over = 0
    winnaar = 0
    current_player = 1
    for z in range(0, 16):
        playingfield[z] = 0
    cleandisplay()


# handle the keypress
def on_key(key_index, pressed):
    global game_over
    global current_player
    if pressed and not game_over:
        if playingfield[key_index] == 0:
            playingfield[key_index] = current_player

            if current_player == 1:
                current_player = 2
            elif current_player == 2:
                current_player = 1

            drawplayingfield()
            checkplayingfield()
            if game_over == 1:
                drawwinningfield()


# add the key interupt
keypad.add_handler(on_key)