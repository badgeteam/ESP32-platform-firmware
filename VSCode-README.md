# Visual Studio code (aka VSCode) and badge.team ESP32 Firmware

VSCode is a lightweight 'IDE' option to develop for the badge firmware. It isn't perfect but with the included configuration the basics should work good enough.
This setup assumes you use Linux but MacOS should work the same. Under windows VSCode should also work without issues but the badge development setup doesn't include windows configuration.
If you really want to use windows you have to fix the compiler etc yourself but the c_cpp_properties.json should work if you correct the paths.

## Setup

1.  Download VSCode from <https://code.visualstudio.com/>
2.  Install VSCode for your operating system
3.  Open VSCode and go to View->Extensions
4.  Install the 'C/C++' extension from microsoft, this includes IntelliSense functionality for code checking and code completion
5.  Optional: install the 'Python' extension from microsoft.
6.  Restart VSCode
7.  Checkout the master branch of the ESP32 Firmware if you haven't done so and open it with: File->Open Folder
8.  Follow the general instructions in the README.md
9.  You are ready to code :)

## Issues

Not everything works perfect so there are a couple of things to keep in mind:

-   VSCode supports the .clang-format for code formatting but it only supports an older version, if code formatting doesnt work or throws errors you might want to comment some options (Like: AlignConsecutiveMacros)
-   VSCode terminal works fine for git actions, build.sh and flash.sh but it can give problems with more advanced terminal output. If you use monitor.sh for the output of the badge menu or Python shell than the output can go haywire. Just use a normal terminal window to use monitor.sh
-   If you get errors or code that cant be resolved from includes first try to run build.sh (Maybe the sdkconfig.h is out-of-date), if that doesn't help you might need to restart VSCode if it doesnt pickup the changes. And if that doesn't work you might to add a path to c_cpp_properties.json config