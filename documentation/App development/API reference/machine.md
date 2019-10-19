---
title: "Machine"
nodateline: true
weight: 40
---

The *machine* API is a standard micropython module which the badge.team has extended with non-volatile storage functions which are for example used to store the WiFi settings.

# Reference

| Command       | Parameters          | Description                            |
| ------------- | ------------------  | -------------------------------------- |
| nvs_set_u8    | \[key\], \[value\]  | Set a u8 value in the given key        |
| nvs_get_u8    | \[key\]             | Get a u8 value for the given key       |
| nvs_set_u16   | \[key\[, \[value\]  | Set a u16 value in the given key       |
| nvs_get_u16   | \[key\]             | Get a u16 value for the given key      |
| nvs_setint    | \[key\], \[value\]  | Set an integer value for the given key | 
| nvs_getint    | \[key\]             | Get an integer value for the given key |
| nvs_setstr    | \[key\], \[value\]  | Set a string for the given key         |
| nvs_getstr    | \[key\]             | Get a string for the given key         |
| nvs_erase     | \[key\]             | Remove the key entry from nvs          |
| nvs_erase_all | \-                  | Remove all entries from nvs            |

# Examples

## Starting an app

```
import machine
machine.nvs_set_str("owner", "name", "badge.team")
```
