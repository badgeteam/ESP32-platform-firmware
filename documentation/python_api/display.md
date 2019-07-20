# The display module
This module is available on platforms which have the framebuffer driver enabled.

## A note about colors
Colors are always represented as 24-bit from the Python userland, but devices with a smaller colorspace will not actually store the exact color information provided.
Even if you have a black and white display then 0x000000 is black and 0xFFFFFF is white.

## Usage
| Command           | Parameters                                          | Description                                                                     | Example                                                 |
|-------------------|-----------------------------------------------------|---------------------------------------------------------------------------------|---------------------------------------------------------|
| flush             | *None*                                              | Send the framebuffer to the display                                             | display.flush()                                         |
| fill              | color                                               | Fill the framebuffer with a color                                               | display.fill(0xFFFFFF)                                  |
| font              | font-name                                           | Set the font to use for text operations                                         | display.font("freesans9")                               |
| cursor            | [x], [y]                                            | Get or set the cursor for text operations                                       | display.cursor(0,0) (x, y) = display.cursor()           |
| textColor         | color                                               | Get or set the color for text operations                                        | display.textColor(0x000000) color = display.textColor() |
| print             | text                                                | Print text to the framebuffer                                                   | display.print("Hello, world")                           |
| get_string_width  | text                                                | Get the width a print operation would need to draw the text                     | width = display.get_string_width("Hello, world")        |
| get_string_height | text                                                | Get the height a print operation would need to draw the text                    | height = display.get_string_height("Hello, world")      |
| rect              | x, y, width, height, fill, color                    | Draw a rectangle                                                                | display.rect(10,10,10,10,True,0x000000)                 |
| circle            | x, y, radius, start_angle, end_angle, fill, color   | Draw a circle                                                                   | display.circle(10,10,10,0,359,True,0x000000)            |
| line              | x0, y0, x1, y1, color                               | Draw a line                                                                     | display.line(30,30,40,40,0x000000)                      |
| greyscale         | enable                                              | Enable or disable greyscale mode on E-ink displays (Ignored on other platforms) | display.greyscale(True)                                 |
