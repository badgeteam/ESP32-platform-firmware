import defines, rgb, buttons, system, uinterface
from random import randint
from time import sleep


UP, DOWN, LEFT, RIGHT = defines.BTN_UP, defines.BTN_DOWN, defines.BTN_LEFT, defines.BTN_RIGHT

can_move = True
cur_direction = RIGHT
next_direction = RIGHT
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
    global next_direction
    global can_move
    if pressed and cur_direction != DOWN:
        next_direction = UP
        can_move = False


def input_down(pressed):
    global next_direction
    global can_move
    if pressed and cur_direction != UP:
        next_direction = DOWN
        can_move = False


def input_left(pressed):
    global next_direction
    global can_move
    if pressed and cur_direction != RIGHT:
        next_direction = LEFT
        can_move = False


def input_right(pressed):
    global next_direction
    global can_move
    if pressed and cur_direction != LEFT:
        next_direction = RIGHT
        can_move = False


def input_B(pressed):
    global next_direction
    next_direction = defines.BTN_B


buttons.register(UP, input_up)
buttons.register(DOWN, input_down)
buttons.register(LEFT, input_left)
buttons.register(RIGHT, input_right)
buttons.register(defines.BTN_B, input_B)

while next_direction != defines.BTN_B:

    cur_x, cur_y = snake[0]
    new_x, new_y = (cur_x + (1 if next_direction == RIGHT else (-1 if next_direction == LEFT else 0)),
                     cur_y + (1 if next_direction == DOWN else (-1 if next_direction == UP else 0)))

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
    # Sleeping 10 times instead of 1 large chunk, for better button responsiveness
    [sleep(0.01) for i in range(0,10)]
    cur_direction = next_direction

rgb.clear()
uinterface.skippabletext("Score - " + str(score))
system.reboot()