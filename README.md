Parrot
======

####description

On install Parrot runs as a systemd unit daemon and made available as a dbus
service.  Parrot's dbus service offers signals and methods that will aid in
backing up files or signalling on events.

![ScreenShot](/screenshots/parrot_dfeet.img)

The screenshot above shows an instance of Parrot on the d-feet (a dbus utility 
program).  Once a Parrot watch is set on a directory, anytime a file in that 
directory is opened Parrot will make a backup of that file before any changes
occur.  That way if any mistakes are written to that file and saved, there will
be a backup of the file before those changes took place.

####usage


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
