# Configuration code library and test spec

## Target system Raspbery Pi Pico
## Language c

## Overview

To create a command line interface (CLI) using the usb port as a serial interface
The CLI is used to enumerate and set arbitary variable constants witin an area of the NVRam of the Pico
The library allow access from a program to the constants.
The constants are defined in "config.h"
all strings are held in language arrays, with the variable name indicating the language useing the two letter international code.
it shall be possible to change languages at the cli.
The NV RAM rea shall be checksummed, if the checksum is not valid, it is assumed that the area is uninitialised.

## CLI commands
? Print welcome string (TBD)
c Clear NV RAM - Ask for confiirmation y/n
e enumerate all constants
q quit
s set all constants by prompting in order
s <constant name> set a particular constant
w write all updated constants and quit

If <constant name> contains "password" then enumeration only emits "****"

# Create a test app to test the cli
the test constants are:
    bool first_run initially true
    string ssid initially blank
    string password initially blank
