#!/usr/bin/env python
# -*- coding: utf-8 -*-

import struct
import socket

#
#  Structure describing an inotify event.
#
#  struct inotify_event
#  {
#    int wd;
#    uint32_t mask;
#    uint32_t cookie;
#    uint32_t len;
#    char name __flexarr;
#  };
#
# 

class ParrotClient(object):

    def __init__(self, server_path):
        self.server_path = server_path
        self.client = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

    def connect(self):
        try:
            self.client.connect(self.server_path)
            self.read_events()
        except socket.error, e:
            print "Error: %s" % e
        
    def read_events(self):
        while True:
            event = self.client.recv(32768)
            if event:
                print "Event received!"
                wd, mask, cookie, namelen = struct.unpack('iIII', event[:16])
                print event[16:namelen + 16]
            else:
                break
        print "Connection closed!"
        self.client.close()
        return


if __name__ == "__main__":
    server_path = '/tmp/p_server'
    parrot_client_test = ParrotClient(server_path)
    parrot_client_test.connect()
