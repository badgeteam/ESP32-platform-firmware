---
title: "System"
nodateline: true
weight: 40
---

The *system* API allows you to control basic features your app needs to provide a smooth experience to the user.

# Reference
| Command        | Parameters                                   | Description                                                                                                                                                                                                             |
| -------------- | -------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| reboot         | \-                                           | Reboot the badge into the currently running app                                                                                                                                                                         |
| sleep          | \[duration\], \[status\]                     | Start sleeping forever or for the provided duration (in seconds). By defaut the function shows the fact that the badge is sleeping on the serial console, this can be disabled by setting the status argument to False. |
| start          | app, \[status\]                              | Start an app. Optionally shows that an app is being started on the screen and in the serial console, for this to happen the status variable must be set to True.                                                        |
| home           | \[status\]                                   | Start the splash screen / default application. To show a message to the user set the optional status flag to True.                                                                                                      |
| launcher       | \[status\]                                   | Start the application launcher. To show a message to the user set the optional status flag to True.                                                                                                                     |
| shell          | \[status\]                                   | Start a raw Python REPL prompt. To show a message to the user set the optional status flag to True.                                                                                                                     |
| ota            | \[status\]                                   | Initiate an Over-The-Air update session. Does NOT check if a newer firmware is available. To prevent hijacking other peoples badges it is NOT possible to provide a different update server or URL at this time.        |
| serialWarning  | \-                                           | Show a message telling the user that the currently running app can only be controlled over the USB-serial connection.                                                                                                   |
| crashedWarning | \-                                           | Show a message telling the user that the currently running app has crashed.                                                                                                                                             |
| isColdBoot     | \-                                           | Returns True if the badge was started from RESET state. This function will only ever return true if the currently runing app was set as the default app.                                                                |
| isWakeup       | \[timer\], \[button\], \[infrared\], \[ulp\] | Returns True if the badge was started from a WARM state. Normally this can be any warm state, however by setting the parameters specific wake reasons can be selected or ruled-out.                                     |
| currentApp     | \-                                           | Returns the name of the currently running app.                                                                                                                                                                          |

# Examples

## Starting an app

```
import system
system.start("2048") # Start the 2048 app (fails if this app has not been installed)
```

## Going back to the launcher

```
import system
system.launcher()
```

## Going back to the homescreen

```
import system
system.home()
```

## Restarting the current app

```
import system
system.reboot()
```

## Sleep for 60 seconds, then return to the current app
```
import system
system.sleep(60000)
```

## Querying the name of the currently runnig app
```
import system
appName = system.currentApp()
if not appName:
	print("This code is running either in the shell or in the boot context")
else:
	print("Currently running app: "+appName)
```
