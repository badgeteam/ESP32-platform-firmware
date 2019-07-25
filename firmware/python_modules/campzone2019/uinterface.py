import rgb, buttons, defines, time, wifi
from default_icons import icon_no_wifi, animation_loading, animation_connecting_wifi

ACTION_NO_OPERATION = 0
ACTION_CONFIRM = 2
ACTION_CANCEL = 4

DEFAULT_CHARSET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"

FONT = rgb.FONT_7x5
FONT_WIDTH = 5

CONFIRMATION_YES_IMAGE = {
    "data": [0, 0, 0, 0x00FF0000, 0x00FF0000, 0, 0x00FF0000, 0, 0, 0x00FF0000, 0, 0, 0, 0, 0, 0],
    "width": 4,
    "height": 4
}

CONFIRMATION_NO_IMAGE = {
    "data": [0xFF000000, 0, 0, 0xFF000000, 0, 0xFF000000, 0xFF000000, 0, 0, 0xFF000000, 0xFF000000, 0, 0xFF000000, 0, 0, 0xFF000000],
    "width": 4,
    "height": 4
}

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

confirm_dialog_action = ACTION_NO_OPERATION

_menu_user_callback_left = None
_menu_user_callback_right = None

def menu(items, selected = 0, callback_left=None, callback_right=None):
    global _menu_user_callback_left
    global _menu_user_callback_right
    _menu_user_callback_left = callback_left
    _menu_user_callback_right = callback_right
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

def confirmation_dialog(text):
    global confirm_dialog_action
    _confirmation_dialog_initialize(text)

    while confirm_dialog_action == ACTION_NO_OPERATION:
        time.sleep(0.1)
    _input_exit_routine()

    return (confirm_dialog_action == ACTION_CONFIRM)

def connect_wifi():
    if wifi.status():
        return True

    rgb.clear()
    data, size, frames = animation_connecting_wifi
    rgb.framerate(3)
    rgb.gif(data, (12, 0), size, frames)

    wifi.connect()
    wifi.wait()

    if not wifi.status():
        data, frames = icon_no_wifi
        rgb.gif(data, (12, 0), (8, 8), frames)
        time.sleep(3)

    rgb.clear()
    rgb.framerate(20)
    return wifi.status()

def loading_text(text):
    data, size, frames = animation_loading
    rgb.gif(data, (1, 1), size, frames)
    rgb.scrolltext(text, pos=(8,0), width=(rgb.PANEL_WIDTH - 8))

def _menu_init_display_and_state(items, selected):
    global menu_state
    menu_state["items"] = items
    menu_state["selected"] = selected
    menu_state["pressed_button"] = ACTION_NO_OPERATION

    _initialize_display()
    
    _menu_register_callbacks()
    _draw_menu_item(items[selected])
    return menu_state

def _menu_register_callbacks():
    buttons.init_button_mapping()
    buttons.register(defines.BTN_A, _menu_select_callback)
    buttons.register(defines.BTN_B, _menu_back_callback)
    buttons.register(defines.BTN_UP, _menu_up_callback)
    buttons.register(defines.BTN_DOWN, _menu_down_callback)
    buttons.register(defines.BTN_LEFT, _menu_left_callback)
    buttons.register(defines.BTN_RIGHT, _menu_right_callback)

def _text_input_init_display_and_state(charset):
    global text_input_state
    text_input_state["charset"] = charset
    text_input_state["text"] = [charset[0]]
    text_input_state["action"] = ACTION_NO_OPERATION
    text_input_state["cursor"] = 0

    _initialize_display()

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

def _confirmation_dialog_initialize(text):
    global confirm_dialog_action

    confirm_dialog_action = ACTION_NO_OPERATION
    _initialize_display()

    _confirmation_dialog_register_callbacks()
    _draw_confirmation_dialog(text)

def _confirmation_dialog_register_callbacks():
    buttons.init_button_mapping()
    buttons.register(defines.BTN_A, _confirmation_dialog_yes_callback)
    buttons.register(defines.BTN_B, _confirmation_dialog_no_callback)

def _initialize_display():
    rgb.clear()
    rgb.framerate(20)
    rgb.setfont(FONT)

def _input_exit_routine():
    rgb.clear()
    buttons.clear_button_mapping()

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

def _menu_left_callback(pressed):
    global menu_state
    if callable(_menu_user_callback_left):
        _menu_user_callback_left(pressed)
        _menu_back_callback(pressed)


def _menu_right_callback(pressed):
    global menu_state
    if callable(_menu_user_callback_right):
        _menu_user_callback_right(pressed)
        _menu_back_callback(pressed)


def _menu_back_callback(pressed):
    global menu_state
    if pressed:
        menu_state["pressed_button"] = _add_action_state(menu_state["pressed_button"], ACTION_CANCEL)

def _menu_select_callback(pressed):
    global menu_state
    if pressed:
        menu_state["pressed_button"] = _add_action_state(menu_state["pressed_button"], ACTION_CONFIRM)

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
            text_input_state["action"] = _add_action_state(text_input_state["action"], ACTION_CANCEL)

def _text_input_confirm_callback(pressed):
    global text_input_state
    if pressed:
        text_input_state["action"] = _add_action_state(text_input_state["action"], ACTION_CONFIRM)

def _confirmation_dialog_yes_callback(pressed):
    global confirm_dialog_action
    if pressed:
        confirm_dialog_action = _add_action_state(confirm_dialog_action, ACTION_CONFIRM)

def _confirmation_dialog_no_callback(pressed):
    global confirm_dialog_action
    if pressed:
        confirm_dialog_action = _add_action_state(confirm_dialog_action, ACTION_CANCEL)

def _add_action_state(action_state, added_state):
    return action_state | added_state

def _draw_menu_item(item):
    rgb.clear()
    rgb.scrolltext(item)

def _draw_text_input_state(cursor, text):
    rgb.clear()
    before_mid = text[:cursor][-2:]
    after_mid = text[cursor+1:][:2]
    mid = text[cursor]
    step = FONT_WIDTH + 1
    midx = int(rgb.PANEL_WIDTH / 2) - int(step / 2)
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
            rgb.text(chars[i], colour, (curx, 0))
            curx -= step
    else:
        for i in range(length):
            rgb.text(chars[i], colour, (curx, 0))
            curx += step

def _draw_confirmation_dialog(text):
    yes_x, yes_y = (rgb.PANEL_WIDTH - CONFIRMATION_YES_IMAGE["width"]), 0
    no_x, no_y = (rgb.PANEL_WIDTH - CONFIRMATION_NO_IMAGE["width"]), CONFIRMATION_YES_IMAGE["height"]
    scroll_offset = max(CONFIRMATION_YES_IMAGE["width"], CONFIRMATION_NO_IMAGE["width"])
    _draw_image(CONFIRMATION_YES_IMAGE, yes_x, yes_y)
    _draw_image(CONFIRMATION_NO_IMAGE, no_x, no_y)
    rgb.scrolltext(text, pos=(0,0), width=(rgb.PANEL_WIDTH - scroll_offset))

def _draw_image(image, x, y):
    rgb.image(
        image["data"], 
        (x, y), 
        (image["width"], 
        image["height"])
        )