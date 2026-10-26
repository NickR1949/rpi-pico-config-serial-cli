<H>A config Test </h>
Create an application to read and write vsariables from the usb port and store them in NV ram.
create a 10kb nvram area to store arbitary variables.
create subroutines to read and write the arbitary variables
the variables are:
    bool first_run initially true
    string ssid initially blank
    string password initially blank

it shall be possible to enumerate the variables except for password where *'s are emitted
an initial prompt shall be a list of all stored variables
pressing enter shall prompt for each variable in turn, except for first_run which will always be updated to false on completion of the input task.
