import display, system, pca9555

def draw():
    display.drawFill(0x000000)
    text  = "Buttons:\n"
    text += " - Home:    " + ("Pressed" if btnHome   else "Released") + "\n"
    text += " - Menu:    " + ("Pressed" if btnMenu   else "Released") + "\n"
    text += " - Start:   " + ("Pressed" if btnStart  else "Released") + "\n"
    text += " - Select:  " + ("Pressed" if btnSelect else "Released") + "\n"
    text += " - Accept:  " + ("Pressed" if btnAccept else "Released") + "\n"
    text += " - Back:    " + ("Pressed" if btnBack   else "Released") + "\n"
    text += "\nJoystick: \n"
    text += " - Left:    " + ("Pressed" if joyLeft   else "Released") + "\n"
    text += " - Right:   " + ("Pressed" if joyRight  else "Released") + "\n"
    text += " - Up:      " + ("Pressed" if joyUp     else "Released") + "\n"
    text += " - Down:    " + ("Pressed" if joyDown   else "Released") + "\n"
    text += " - Pressed: " + ("Pressed" if joyPress  else "Released") + "\n"
    display.drawText(0,0,text, 0xFFFFFF, "ocra16")
    display.flush()
    
def a(p):
    global joyLeft
    joyLeft = p
    draw()
def b(p):
    global joyPress
    joyPress = p
    draw()
def c(p):
    global joyDown
    joyDown = p
    draw()
def d(p):
    global joyUp
    joyUp = p
    draw()
def e(p):
    global joyRight
    joyRight = p
    draw()
def start(p):
    global btnStart
    btnStart = p
    draw()
def select(p):
    global btnSelect
    btnSelect = p
    draw()
def home(p):
    global btnHome
    btnHome = p
    draw()
def menu(p):
    global btnMenu
    btnMenu = p
    draw()
def accept(p):
    global btnAccept
    btnAccept = p
    draw()
def back(p):
    global btnBack
    btnBack = p
    draw()

btnStart  = pca9555.value(5)
btnSelect = pca9555.value(6)
btnMenu   = pca9555.value(7)
btnHome   = pca9555.value(8)
joyLeft   = pca9555.value(9)
joyPress  = pca9555.value(10)
joyDown   = pca9555.value(11)
joyRight  = pca9555.value(12)
joyUp     = pca9555.value(13)
btnBack   = pca9555.value(14)
btnAccept = pca9555.value(15)

pca9555.attach(5, start)
pca9555.attach(6, select)
pca9555.attach(7, menu)
pca9555.attach(8, home)
pca9555.attach(9, a)
pca9555.attach(10, b)
pca9555.attach(11, c)
pca9555.attach(12, d)
pca9555.attach(13, e)
pca9555.attach(14, back)
pca9555.attach(15, accept)
    
draw()
