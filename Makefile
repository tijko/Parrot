CC = gcc
TARGET = parrot

LOGDIR = src/logging
DBUSDIR = src/dbus
CDIR = $(shell pwd)

POBJ = $(CDIR)/src/dbus/parrot_object.h
INTRO = $(CDIR)/introspection/parrot.xml

SRC = $(wildcard src/*.c)
LOG = $(wildcard $(LOGDIR)/*.c)
DBUS = $(wildcard $(DBUSDIR)/*.c)

SD = $(shell pkg-config --cflags --libs libsystemd-journal)
GLIB = $(shell pkg-config --cflags --libs glib-2.0)
DB = $(shell pkg-config --cflags --libs dbus-1)
GB = $(shell pkg-config --cflags --libs dbus-glib-1)

FLAGS = -g -lpthread $(SD) -I./src -Wall $(GB) $(GLIB) $(DB) 

DBUS_STUB := $(shell dbus-binding-tool --prefix=parrot_obj --mode=glib-server \
                     --output=$(POBJ) $(INTRO))

$(TARGET): $(TARGET).c $(SRC) $(LOG) $(DBUS)
	$(CC) $(TARGET).c $(SRC) $(LOG) $(DBUS) -o $(TARGET) $(FLAGS)

install:
	cp $(TARGET) /usr/bin
	cp $(CDIR)/systemd/dbus.socket /etc/systemd/user/
	cp $(CDIR)/systemd/dbus.service /etc/systemd/user/
	mkdir /etc/systemd/system/user@.service.d/
	cp $(CDIR)/systemd/dbus.conf /etc/systemd/system/user@.service.d/
	cp $(CDIR)/conf/org.parrot.conf /etc/dbus-1/session.d/
	cp $(CDIR)/systemd/parrot.service /etc/systemd/user/
	systemctl --global enable dbus.socket

install-service:
	systemctl --user daemon-reload
	systemctl --user enable parrot.service
	systemctl --user start dbus.socket
	systemctl --user start dbus.service
	systemctl --user start parrot.service

uninstall:
	systemctl --global disable dbus.socket
	rm /etc/systemd/user/dbus.socket
	rm /etc/systemd/user/dbus.service
	rm -r /etc/systemd/system/user@.service.d/
	rm /etc/dbus-1/session.d/org.parrot.conf
	rm /etc/systemd/user/parrot.service
	rm /usr/bin/$(TARGET)
	rm $(POBJ)

uninstall-service:
	systemctl --user disable parrot.service
	systemctl --user stop dbus.socket
	systemctl --user stop dbus.service
	systemctl --user stop parrot.service

clean:
	rm $(POBJ)
	rm $(TARGET)
