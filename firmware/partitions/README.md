# Partition tables
This folder contains the partition tables.

## A bit of history
The SHA2017 and Disobey 2019 badges have been released originally with a partition table reserving space for an over-the-air content distribution system called "bpp" that in the end was never used.

The firmware for both of these badges contains a function called "esp.update_partition_table()" which will replace the partition table with a new variant without the reserved space. Note that this will move the starting point of the FAT partition containing user data, causing the old partition to become inaccessible. This means that all user apps and files will be gone after a reboot of the badge. In return you will get access to the full space available on the SPI flash.

## Flash sizes per device
  - Most chinese devices (& our prototypes): 4MB
  - SHA2017 badge: 16MB
  - Disobey 2019 badge: 16MB
  - HackerHotel 2019 badge: 16MB
  - CampZone 2019 badge: 8MB
  - Disobey 2020 badge: 16MB
