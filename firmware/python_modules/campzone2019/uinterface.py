import rgb, buttons, defines, time

MENU_NO_OPERATION = 0
MENU_SELECT_ITEM = 2
MENU_MOVE_BACK = 4

DEFAULT_CHARSET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"

FONT = rgb.FONT_7x5
FONT_HEIGHT = 7
FONT_WIDTH = 5
SCREEN_WIDTH = 32

menu_state = {
    "selected" : 0,
    "items": [],
    "pressed_button": MENU_NO_OPERATION
}

def menu(items, selected = 0):
    state = _menu_init_display_and_state(items, selected)
    while state["pressed_button"] == MENU_NO_OPERATION:
        time.sleep(0.1)
    _menu_exit_routine()
    if state["pressed_button"] == MENU_SELECT_ITEM:
        return state["selected"]
    else:
        return None

def text_input(charset = DEFAULT_CHARSET):
    pass

def _menu_init_display_and_state(items, selected):
    global menu_state
    menu_state["items"] = items
    menu_state["selected"] = selected
    menu_state["pressed_button"] = MENU_NO_OPERATION
    rgb.clear()
    rgb.framerate(20)
    rgb.setfont(FONT)
    _menu_register_callbacks()
    _draw_menu_item(items[selected])
    return menu_state

def _menu_exit_routine():
    rgb.clear()
    buttons.clear_button_mapping()

def _menu_register_callbacks():
    buttons.init_button_mapping()
    buttons.register(defines.BTN_A, _menu_select_callback)
    buttons.register(defines.BTN_B, _menu_back_callback)
    buttons.register(defines.BTN_UP, _menu_up_callback)
    buttons.register(defines.BTN_DOWN, _menu_down_callback)

def _menu_up_callback(pressed):
    global menu_state
    if pressed:
        selected = menu_state["selected"]
        item_count = len(menu_state["items"])
        selected -= 1
        if (selected < 0):
            selected = item_count - 1

        menu_state["selected"] = selected
        _draw_menu_item(menu_state["items"][selected])

def _menu_down_callback(pressed):
    global menu_state
    if pressed:
        selected = menu_state["selected"]
        item_count = len(menu_state["items"])
        selected += 1
        if (selected >= item_count):
            selected = 0

        menu_state["selected"] = selected
        _draw_menu_item(menu_state["items"][selected])

def _menu_back_callback(pressed):
    if pressed:
        _menu_add_button_press_state(MENU_MOVE_BACK)

def _menu_select_callback(pressed):
    if pressed:
        _menu_add_button_press_state(MENU_SELECT_ITEM)

def _menu_add_button_press_state(button_state):
    global menu_state
    menu_state["pressed_button"] = menu_state["pressed_button"] | button_state

def _draw_menu_item(item):
    rgb.clear()
    rgb.scrolltext(item)