Parrot
======

Parrot is a service that backs up files from a monitored file/directory.

####description

Parrot runs as a systemd user service and also as a dbus service.  Parrot's dbus 
service capabilities offer signals and methods that allow you to remotely set 
up watches to back up files and/or signal on events.

Parrot is a systemd _user_ service, being so means that the 
`DBUS_SESSION_BUS_ADDRESS` will not be set to the correct bus address (since 
none of the environment variables are set in a systemd user instance).  To 
circumvent this, Parrot runs an instance of dbus as a systemd service.  There 
are several files under the `systemd` directory to set this up.  The `.socket` 
unit activates the systemd socket for the dbus bus address.  The matching 
`.service` unit executes the dbus session, while the `.conf` drop in sets the 
environment variable bus address name.

![ScreenShot](/screenshots/parrot_dfeet.img)

The screenshot above shows an instance of Parrot on d-feet (a dbus utility 
program).  Once a Parrot watch is set on a directory, anytime a file in that 
directory is opened Parrot will make a backup of that file before any changes
occur.  That way if any mistakes are written to that file and saved, there will
be a backup of the file before those changes took place.

In addition to backing up files, Parrot functions in helping log when those 
changes/accesses in the Parrot'd directory occur.  By subscribing to Parrot's
`accessed` signal, Parrot will broadcast a signal in the form of seconds since
the epoch whenever a file in the watch directory is accessed.  This signal can
easily be logged in whatever form needed or some other form of action can be 
take place such as callbacks to extend any additional functionality that might
be called for.

####setup

Edit the `parrot.h` header file with the path to the directory you 
wish to backup to.

Be sure to include a trailing `/` on the pathname:

    /home/user/path/to/backup_to/ <-- here

####install

GCC is required to compile, to install run in the base directory:

    [user@user ~]$ make
    [user@user ~]$ sudo make install
    [user@user ~]$ make install-service
    [user@user ~]$ make clean

and to uninstall:

    [user@user ~]$ sudo make uninstall
    [user@user ~]$ make uninstall-service
