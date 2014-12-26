Parrot
======

On install Parrot runs as a systemd unit daemon and made available as a dbus
service.  Parrots dbus services offer signals and methods that will aid in
backing up files or signalling on events.

####setup

Edit the `parrot.h` header file with the path to the directory or file you 
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
