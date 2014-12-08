#!/usr/bin/env python
# -*- coding: utf-8 -*-

import dbus
from gi.repository import GObject
from dbus.mainloop.glib import DBusGMainLoop


def callback(msg):
    print msg

def parrot_proxy():
    bus = dbus.SessionBus(mainloop=DBusGMainLoop())
    proxy = bus.get_object('org.Parrot', '/org/Parrot')
    proxy.connect_to_signal('accessed', callback, dbus_interface='org.Parrot.Inotify')
    loop = GObject.MainLoop()
    loop.run()


if __name__ == "__main__":
    parrot_proxy()
