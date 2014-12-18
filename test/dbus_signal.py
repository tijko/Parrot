#!/usr/bin/env python
# -*- coding: utf-8 -*-

import time

import dbus.service
from gi.repository import GObject
from dbus.mainloop.glib import DBusGMainLoop


def callback(access_time):
    print time.ctime(access_time)

def parrot_proxy():
    conn = dbus.bus.BusConnection(
                    'unix:path=/run/user/1000/dbus/user_bus_socket',
                     mainloop=DBusGMainLoop()
                                 )
    bus = dbus.service.BusName('org.Parrot', conn)
    proxy = conn.get_object(bus.get_name(), '/org/Parrot')
    proxy.connect_to_signal('accessed', callback, 
                            dbus_interface='org.Parrot.Inotify')

    watch_method = proxy.get_dbus_method('current_watch')
    add_watch = proxy.get_dbus_method('add_watch')
    print "Calling 'add_watch' method..."
    add_watch('/home/tijko/vim-profile/')
    print "Calling 'current_watch' method..."
    cur_watch = watch_method() 
    print "Currently watched: %s" % cur_watch

    print "Waiting on signal 'accessed'..."
    
    loop = GObject.MainLoop()
    loop.run()


if __name__ == "__main__":
    parrot_proxy()
