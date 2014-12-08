Parrot
======

Parrot runs as a daemon catching any write events on a specific file or 
directory and copys those files to a backup directory.

####setup

Edit the `interface.h` header file with the path to the directory or file you 
wish to Parrot and the path to where you want to backup to.

Be sure to include a trailing `/` on the pathname:

    /home/user/path/to/backup/ <-- here

on both pathnames, the backup pathname and the directory being backed up to.

####install

GCC is required to compile, to install run in the base directory:

    [user@user ~]$ make
    [user@user ~]$ sudo make install
    [user@user ~]$ make clean

and to uninstall:

    [user@user ~]$ sudo make uninstall
