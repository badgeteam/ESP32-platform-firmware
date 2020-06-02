## stdin / stdout hooking

This driver enables changes in micropython's mphalport.c, such that it writes data to an reads data from custom Virtual File System (VFS) mount points if they exist.

A driver can register these VFS mount points, and then use them to read from micropython's stdout, and write into its stdin.

For example usage, see driver_fsoveruart. That uses this concept to pipe stdin/stdout over WebUSB.