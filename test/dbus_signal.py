#!/usr/bin/env python
# -*- coding: utf-8 -*-

import dbus.service
from gi.repository import GObject
from dbus.mainloop.glib import DBusGMainLoop


def callback(msg):
    print msg

def parrot_proxy():
    conn = dbus.bus.BusConnection(
                    'unix:path=/run/user/1000/dbus/user_bus_socket',
                     mainloop=DBusGMainLoop()
                                 )
    bus = dbus.service.BusName('org.Parrot', conn)
    proxy = conn.get_object(bus.get_name(), '/org/Parrot')
    proxy.connect_to_signal('accessed', callback, 
                            dbus_interface='org.Parrot.Inotify')
    loop = GObject.MainLoop()
    loop.run()


if __name__ == "__main__":
    parrot_proxy()
