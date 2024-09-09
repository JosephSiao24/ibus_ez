CC = gcc
CFLAGS += `pkg-config --cflags dbus-glib-1 glib-2.0`
LIBS += `pkg-config --libs dbus-glib-1 glib-2.0`
all: dbus_test

dbus_test : dbus_test.c
	$(CC) -Wall -g $(CFLAGS) $< $(LIBS) -o $@
