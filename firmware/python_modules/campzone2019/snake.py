import defines, rgb, buttons, system
from random import randint
from time import sleep

UP, DOWN, LEFT, RIGHT = defines.BTN_UP, defines.BTN_DOWN, defines.BTN_LEFT, defines.BTN_RIGHT

can_move = True
direction = RIGHT
score = 0

snake = [(10, 4), (8, 4), (9, 4)]
food = (24, 6)

def render():
    rgb.clear()
    rgb.background((0,0,0))

    for x,y in snake:
        rgb.pixel((255, 255, 255), (x, y))

    rgb.pixel((0, 255, 200), food)


def input_up(pressed):
    global direction
    global can_move
    if direction != DOWN and can_move:
        direction = UP
        can_move = False


def input_down(pressed):
    global direction
    global can_move
    if direction != UP and can_move:
        direction = DOWN
        can_move = False


def input_left(pressed):
    global direction
    global can_move
    if direction != RIGHT and can_move:
        direction = LEFT
        can_move = False


def input_right(pressed):
    global direction
    global can_move
    if direction != LEFT and can_move:
        direction = RIGHT
        can_move = False


def input_B(pressed):
    global direction
    direction = defines.BTN_B


buttons.register(UP, input_up)
buttons.register(DOWN, input_down)
buttons.register(LEFT, input_left)
buttons.register(RIGHT, input_right)
buttons.register(defines.BTN_B, input_B)

while direction != defines.BTN_B:

    cur_x, cur_y = snake[0]
    new_x, new_y = (cur_x + (1 if direction == RIGHT else (-1 if direction == LEFT else 0)),
                     cur_y + (1 if direction == DOWN else (-1 if direction == UP else 0)))

    # Make snake loop from one border to the other
    new_x %= rgb.PANEL_WIDTH
    new_y %= rgb.PANEL_HEIGHT
    snake.insert(0, (new_x, new_y))

    # If snake bites itself, the game's over
    if snake[0] in snake[1:]:
        print('dead')
        break

    if snake[0] == food:
        # Snake eats the food
        food = []
        score += 1

        # Spawn new food
        while food == []:
            food = (randint(0, rgb.PANEL_WIDTH-1), randint(0, rgb.PANEL_HEIGHT-1))
            if food in snake: food = []
    else:
        # Remove last entry from tail (snake didn't grow)
        last = snake.pop()

    render()
    sleep(0.2)
    can_move = True

rgb.clear()
rgb.scrolltext("Score - " + str(score))
sleep(4)
system.reboot()