#include <stdio.h>
#include<dbus/dbus-glib.h>

int main(void){
	GError *error;
	DBusGConnection *conn;
	DBusGProxy *proxy;
	char *str;
	
	error = NULL;
	conn = (dbus_g_bus_get(DBUS_BUS_SESSION, &error);
	return 0;
}
