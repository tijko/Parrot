CC = gcc
TARGET = parrot

LOGDIR = src/logging
DBUSDIR = src/dbus
CDIR := $(shell pwd)

POBJ = $(CDIR)/src/dbus/parrot_object.h
INTRO = $(CDIR)/introspection/parrot.xml

SRC := $(wildcard src/*.c)
LOG := $(wildcard $(LOGDIR)/*.c)
DBUS := $(wildcard $(DBUSDIR)/*.c)

#SD := $(shell pkg-config --cflags --libs libsystemd-journal)
SD := -I/usr/include/systemd -I/usr/lib/systemd/ -lsystemd
GLIB := $(shell pkg-config --cflags --libs glib-2.0)
DB := $(shell pkg-config --cflags --libs dbus-1)
GB := $(shell pkg-config --cflags --libs dbus-glib-1)

FLAGS = -g -lpthread $(SD) -I./src -Wall $(GB) $(GLIB) $(DB) 

DBUS_STUB := $(shell dbus-binding-tool --prefix=parrot_obj --mode=glib-server \
                     --output=$(POBJ) $(INTRO))

$(TARGET): $(TARGET).c $(SRC) $(LOG) $(DBUS)
	$(CC) $(TARGET).c $(SRC) $(LOG) $(DBUS) -o $(TARGET) $(FLAGS)

install:
	cp $(TARGET) /usr/bin
	if [[ ! -d /etc/systemd/system/user@.service.d ]]; then \
		mkdir /etc/systemd/system/user@.service.d/; \
	fi
	cp $(CDIR)/systemd/dbus.conf /etc/systemd/system/user@.service.d/
	cp $(CDIR)/conf/org.parrot.conf /etc/dbus-1/session.d/
	cp $(CDIR)/systemd/parrot.service /etc/systemd/user/

install-service:
	systemctl --user daemon-reload
	systemctl --user enable parrot.service
	systemctl --user start parrot.service

uninstall:
	rm -r /etc/systemd/system/user@.service.d/
	rm /etc/dbus-1/session.d/org.parrot.conf
	rm /etc/systemd/user/parrot.service
	rm /usr/bin/$(TARGET)
	rm $(POBJ)

uninstall-service:
	systemctl --user disable parrot.service
	systemctl --user stop parrot.service

clean:
	rm $(POBJ)
	rm $(TARGET)
