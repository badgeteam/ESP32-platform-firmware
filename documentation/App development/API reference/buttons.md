---
title: "Buttons"
nodateline: true
weight: 50
---

The *buttons* API allows you to read the state of the buttons on a badge.
This API encapsulates the drivers for different button types.

# Badge support
This API is currently supported on the following badges:

 - SHA2017
 - Hackerhotel 2019
 - Disobey 2019
 - CampZone 2019
 - Disobey 2020
 - Fri3dcamp 2018

Support for GPIO buttons and touch-buttons via the MPR121 touch controller IC are supported. Touch buttons using the touch button features of the ESP32 can not be used (yet).

# Reference

| Command            | Parameters                 | Description                                                                      |
| ------------------ | -------------------------- | -------------------------------------------------------------------------------- |
| attach             | button, callback function  | Attach a callback to a button                                                    |
| detach             | button                     | Detach a callback from a button                                                  |
| value              | button                     | Get the current value of a button                                                |
| getCallback        | button                     | Get the current callback of a button                                             |
| pushMapping        | [mapping]                  | Switch to a new button mapping                                                   |
| popMapping         | *none*                     | Switch back to the previous button mapping                                       |
| rotate             | degrees                    | Adapt the button layout to an orientation. Accepts 0, 90, 180 and 270 as values. |

# Button availability per badge
| Name   | SHA2017 | Hackerhotel 2019 | Disobey 2019 | CampZone 2019 | Disobey 2020 | Fri3dCamp 2018 | OpenHardwareSummit 2018 |
|--------|---------|------------------|--------------|---------------|--------------|----------------|-------------------------|
| A      | Yes     | Yes              | Yes          | Yes           | Yes          |                |                         |
| B      | Yes     | Yes              | Yes          | Yes           | Yes          |                |                         |
| SELECT | Yes     | Yes              | No           | No            | Yes          |                |                         |
| START  | Yes     | Yes              | No           | No            | Yes          |                |                         |
| UP     | Yes     | Yes              | Yes          | Yes           | Yes          |                |                         |
| DOWN   | Yes     | Yes              | Yes          | Yes           | Yes          |                |                         |
| LEFT   | Yes     | Yes              | Yes          | Yes           | Yes          |                |                         |
| RIGHT  | Yes     | Yes              | Yes          | Yes           | Yes          |                |                         |

# Default callback per button
| Name   | SHA2017  | Hackerhotel 2019 | Disobey 2019 | CampZone 2019 | Disobey 2020 | Fri3dCamp 2018 | OpenHardwareSummit 2018 |
|--------|----------|------------------|--------------|---------------|--------------|----------------|-------------------------|
| A      |          |                  |              |               |              |                |                         |
| B      |          |                  | Exit app     | Exit app      |              |                |                         |
| SELECT |          |                  |              |               |              |                |                         |
| START  | Exit app | Exit app         |              |               | Exit app     |                |                         |
| UP     |          |                  |              |               |              |                |                         |
| DOWN   |          |                  |              |               |              |                |                         |
| LEFT   |          |                  |              |               |              |                |                         |
| RIGHT  |          |                  |              |               |              |                |                         |