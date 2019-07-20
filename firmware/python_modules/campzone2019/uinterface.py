import rgb, buttons, defines, time, hub75

ACTION_NO_OPERATION = 0
ACTION_CONFIRM = 2
ACTION_CANCEL = 4

DEFAULT_CHARSET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"

FONT = rgb.FONT_7x5
FONT_WIDTH = 5
SCREEN_WIDTH = 32

menu_state = {
    "selected" : 0,
    "items": [],
    "pressed_button": ACTION_NO_OPERATION

}

text_input_state = {
    "cursor": 0,
    "charset": DEFAULT_CHARSET,
    "text": [ DEFAULT_CHARSET[0] ],
    "action": ACTION_NO_OPERATION
}

def menu(items, selected = 0):
    state = _menu_init_display_and_state(items, selected)
    while state["pressed_button"] == ACTION_NO_OPERATION:
        time.sleep(0.1)
    _input_exit_routine()
    if state["pressed_button"] == ACTION_CONFIRM:
        return state["selected"]
    else:
        return None

def text_input(charset = DEFAULT_CHARSET):
    state = _text_input_init_display_and_state(charset)
    while state["action"] == ACTION_NO_OPERATION:
        time.sleep(0.1)
    _input_exit_routine()
    if state["action"] == ACTION_CONFIRM:
        return "".join(state["text"])
    else:
        return ""

def _text_input_init_display_and_state(charset):
    global text_input_state
    text_input_state["charset"] = charset
    text_input_state["text"] = [charset[0]]
    text_input_state["action"] = ACTION_NO_OPERATION
    text_input_state["cursor"] = 0

    rgb.clear()
    rgb.framerate(20)
    rgb.setfont(FONT)

    _text_input_register_callbacks()
    _draw_text_input_state(text_input_state["cursor"], text_input_state["text"])
    return text_input_state

def _text_input_register_callbacks():
    buttons.init_button_mapping()
    buttons.register(defines.BTN_A, _text_input_confirm_callback)
    buttons.register(defines.BTN_B, _text_input_cancel_callback)
    buttons.register(defines.BTN_UP, _text_input_up_callback)
    buttons.register(defines.BTN_DOWN, _text_input_down_callback)
    buttons.register(defines.BTN_LEFT, _text_input_left_callback)
    buttons.register(defines.BTN_RIGHT, _text_input_right_callback)

def _menu_init_display_and_state(items, selected):
    global menu_state
    menu_state["items"] = items
    menu_state["selected"] = selected
    menu_state["pressed_button"] = ACTION_NO_OPERATION

    rgb.clear()
    rgb.framerate(20)
    rgb.setfont(FONT)
    _menu_register_callbacks()
    _draw_menu_item(items[selected])
    return menu_state

def _input_exit_routine():
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
        _menu_add_button_press_state(ACTION_CANCEL)

def _menu_select_callback(pressed):
    if pressed:
        _menu_add_button_press_state(ACTION_CONFIRM)

def _menu_add_button_press_state(button_state):
    global menu_state
    menu_state["pressed_button"] = menu_state["pressed_button"] | button_state

def _text_input_up_callback(pressed):
    global text_input_state
    if pressed:
        cursor = text_input_state["cursor"]
        text = text_input_state["text"]
        charset = text_input_state["charset"]

        index = charset.index(text[cursor]) + 1
        if index >= len(charset):
            index = 0
        
        text[cursor] = charset[index]
        text_input_state["text"] = text
        _draw_text_input_state(cursor, text)

def _text_input_down_callback(pressed):
    global text_input_state
    if pressed:
        cursor = text_input_state["cursor"]
        text = text_input_state["text"]
        charset = text_input_state["charset"]

        index = charset.index(text[cursor]) - 1
        if index < 0:
            index = len(charset) - 1

        text[cursor] = charset[index]
        text_input_state["text"] = text
        _draw_text_input_state(cursor, text)

def _text_input_left_callback(pressed):
    global text_input_state
    if pressed:
        cursor = text_input_state["cursor"]
        text = text_input_state["text"]
        cursor -= 1
        if cursor < 0:
            cursor = 0
        text_input_state["cursor"] = cursor
        _draw_text_input_state(cursor, text)

def _text_input_right_callback(pressed):
    global text_input_state
    if pressed:
        cursor = text_input_state["cursor"]
        text = text_input_state["text"]
        charset = text_input_state["charset"]
        cursor += 1
        if cursor == len(text):
            text.append(charset[0])
        text_input_state["cursor"] = cursor
        text_input_state["text"] = text
        _draw_text_input_state(cursor, text)

def _text_input_cancel_callback(pressed):
    global text_input_state
    if pressed:
        text = text_input_state["text"]
        cursor = text_input_state["cursor"]
        if len(text) > 1:
            text = text[:-1]
            if (cursor == len(text)):
                cursor -= 1
            text_input_state["cursor"] = cursor
            text_input_state["text"] = text
            _draw_text_input_state(cursor, text)
        else:
            _text_input_add_action_state(ACTION_CANCEL)

def _text_input_confirm_callback(pressed):
    if pressed:
        _text_input_add_action_state(ACTION_CONFIRM)

def _text_input_add_action_state(action_state):
    global text_input_state
    text_input_state["action"] = text_input_state["action"] | action_state

def _draw_text_input_state(cursor, text):
    hub75.clear()
    before_mid = text[:cursor][-2:]
    after_mid = text[cursor+1:][:2]
    mid = text[cursor]
    step = FONT_WIDTH + 1
    midx = int(SCREEN_WIDTH / 2) - int(step / 2)
    beforex = midx - step
    afterx = midx + step
    colour_selected = (150, 50, 10)
    colour_unselected = (80, 80, 80)
    _draw_text_input_sequence(beforex, before_mid, colour_unselected, reverse=True)
    _draw_text_input_sequence(midx, mid, colour_selected)
    _draw_text_input_sequence(afterx, after_mid, colour_unselected)

def _draw_text_input_sequence(startx, chars, colour, reverse=False):
    length = len(chars)
    step = FONT_WIDTH + 1
    curx = startx

    if reverse:
        for i in range(length - 1, -1, -1):
            hub75.text(chars[i], colour[0], colour[1], colour[2], curx, 0)
            curx -= step
    else:
        for i in range(length):
            hub75.text(chars[i], colour[0], colour[1], colour[2], curx, 0)
            curx += step

def _draw_menu_item(item):
    rgb.clear()
    rgb.scrolltext(item)