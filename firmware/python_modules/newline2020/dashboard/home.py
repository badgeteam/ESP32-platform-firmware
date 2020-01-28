import touchbuttons, display

print("Welcome to the shell of your ESP32 device!")
print("Type 'import menu' to enter the menu.")


# --- Touch button demo ---
def btn_up_callback(state):
    print('button UP is %s' % ('pressed' if state else 'released'))

touchbuttons.set_callback(touchbuttons.BTN_UP, btn_up_callback)  # You can set callbacks with normal functions
touchbuttons.set_callback(touchbuttons.BTN_LEFT,
                          lambda state: print('button LEFT is %s' % ('pressed' if state else 'released')))  # Or with lambdas

touchbuttons.set_callback(touchbuttons.BTN_RIGHT, lambda state: print('button RIGHT is %s' % ('pressed' if state else 'released')))
touchbuttons.set_callback(touchbuttons.BTN_DOWN, lambda state: print('button DOWN is %s' % ('pressed' if state else 'released')))
touchbuttons.set_callback(touchbuttons.BTN_A, lambda state: print('button A is %s' % ('pressed' if state else 'released')))
touchbuttons.set_callback(touchbuttons.BTN_B, lambda state: print('button B is %s' % ('pressed' if state else 'released')))
touchbuttons.set_callback(touchbuttons.BTN_START, lambda state: print('button START is %s' % ('pressed' if state else 'released')))


# --- Display demo ---
try:
    display.windowCreate('myWin', 128, 240)
    display.windowShow('myWin')
    display.drawFill('myWin', 0x00C0C0)
    display.drawText('myWin', 0, 0, 'Hello world', 0xFFFFFF, 'permanentmarker22')
    display.flush()
except Exception as e:
    print(e)
    pass