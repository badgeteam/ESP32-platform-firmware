---
title: "Getting started"
nodateline: true
weight: 5
---

# Connecting to your badge
Our badges all emulate a serial port when connected over USB. Depending on your OS you might need to install a driver. Please check the table down below to see what driver you need for your situation.

| Badge                | USB-serial bridge chip | Windows                        | Mac                            | Linux                           |
|----------------------|------------------------|--------------------------------|--------------------------------|---------------------------------|
| SHA2017              |                        | <i>No guides available yet</i> | <i>No guides available yet</i> | No driver installation needed   |
| HackerHotel 2019     |                        | <i>No guides available yet</i> | <i>No guides available yet</i> | No driver installation needed   |
| CampZone 2019 I-PANE | CH340                  | <i>No guides available yet</i> | <i>No guides available yet</i> | No driver installation needed   |
| Disobey 2019         | Arduino / SAMD         | <i>No guides available yet</i> | <i>No guides available yet</i> | No driver installation needed   |
| Disobey 2020         |                        | <i>No guides available yet</i> | <i>No guides available yet</i> | No driver installation needed   |

Once the driver is installed you can communicate with your badge using multiple tools. The following sections will describe the options and uses.

## Terminal
The first way to connect to your badge is using a terminal emulation program to connect to your badge directly. This will give you access to both a menu and the Micropython prompt.

## MicroPython tools

 - [Adafruit MicroPython Tool (ampy)](ampy)
 - [Jupiter Notebook](jupyter-notebook)
