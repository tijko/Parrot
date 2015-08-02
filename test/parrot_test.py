#!/usr/bin/env python
# -*- coding: utf-8 -*-

import time
import os

import dbus.service
from gi.repository import GObject
from dbus.mainloop.glib import DBusGMainLoop


DIR_MASK = 0x1
FIL_MASK = 0x2

TEST_MASK = FIL_MASK

curpath = os.getcwd()


def create_testfile():
    with open(os.path.join(curpath, 'parrot_test_file'), 'w+') as f:
        f.write('parrot test file...')

def create_testdir():
    testdir = os.path.join(curpath, 'parrot_test_dir')
    os.mkdir(testdir)
    
def callback(access_time):
    print time.ctime(access_time)

def parrot_proxy():
    test_path = os.path.join(curpath, 'parrot_test_file')
    test_dir = os.path.join(curpath, 'parrot_test_dir/')
    create_testfile()
    create_testdir()
    conn = dbus.bus.BusConnection(
                    'unix:path=/run/user/1000/dbus/user_bus_socket',
                     mainloop=DBusGMainLoop()
                                 )
    bus = dbus.service.BusName('org.Parrot', conn)
    proxy = conn.get_object(bus.get_name(), '/org/Parrot')
    proxy.connect_to_signal('accessed', callback, 
                            dbus_interface='org.Parrot.Inotify')

    watch_method = proxy.get_dbus_method('current_watches')
    add_watch = proxy.get_dbus_method('add_watch')
    print "Calling 'add_watch' method..."
    add_watch(test_path, test_dir, TEST_MASK)
    print "Calling 'current_watch' method..."
    cur_watch = watch_method() 
    print "Currently watched: %s" % cur_watch
    print "Waiting on signal 'accessed'..."
    loop = GObject.MainLoop()
    loop.run()


if __name__ == "__main__":
    parrot_proxy()
