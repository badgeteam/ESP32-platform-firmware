import rgb, badge, time

# Functions
def msg_nosplit(message, title = 'Loading...', reset = False):
    """Show a terminal style loading screen with title

    title can be optionaly set when resetting or first call
    """
    global messageHistory

    try:
        messageHistory
        if reset:
            raise exception
    except:
        messageHistory = []

    if len(messageHistory)<3:
        messageHistory.append(message)
    else:
        messageHistory.pop(0)
        messageHistory.append(message)
        for i, message in enumerate(messageHistory):
            rgb.clear()
            rgb.scrolltext(message)
            time.sleep(3)
            pass


def msg(message, title = None, reset = False, wait = 0):
    try:
        rgb.clear()
        rgb.scrolltext(message, 255, 255, 255, 0, 0, 32)
        time.sleep(wait)
    except BaseException as e:
        print("!!! Exception in easydraw.msg !!!")
        print(e)

def nickname(y = 0, font = None, color = None):
    nick = badge.nvs_get_str("owner", "name", 'WELCOME TO DISOBEY')
    rgb.clear()
    rgb.scrolltext(nick)
    time.sleep(3)

def battery(on_usb, vBatt, charging):
    pass

def disp_string_right_bottom(y, s):
    pass