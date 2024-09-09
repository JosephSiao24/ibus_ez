#include <stdio.h>
#include<dbus/dbus-glib.h>

int main(void){
	GError *error;
	DBusGConnection *conn;
	DBusGProxy *proxy;
	char *str;
	
	error = NULL;
	conn = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
	if(conn == NULL){
		g_printerr("Failed to open connection to bus:%s\n", error->message);
		g_error_free(error);
		exit(1);
	}
	proxy = dbus_g_proxy_new_for_name(conn,
			"org.freedesktop.Notifications",
			"/",
			"org.freedesktop.DBus.Introspectable"
			);
	error = NULL;
	if (!dbus_g_proxy_call(proxy, "Introspcet", &error, G_TYPE_INVALID, G_TYPE_STRING, &str, G_TYPE_INVALID)){
		if(error->domain == DBUS_GERROR && error->code == DBUS_GERROR_REMOTE_EXCEPTION){
		
		}else{
			g_printerr("Error:%s\n", error->message);
		}
		g_error_free(error);
		exit(1);
	}
	return 0;
}
