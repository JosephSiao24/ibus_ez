#include <iostream>
#include <ibus.h>
#include "engine.h"
using namespace std;

static IBusBus *bus = NULL;
static IBusFactory * factory = NULL;
//目的只是信号发送时执行ibus_quit
static void ibus_disconnected_cb(IBusBus *bus, gpointer user_data){
	ibus_quit();
}

int main(void){
	ibus_init();	
	bus  = ibus_bus_new();
	g_object_ref_sink(bus);
	g_signal_connect(bus, "disconnected", G_CALLBACK(ibus_disconnected_cb), NULL);
	factory = ibus_factory_new(ibus_bus_get_connection(bus));
	g_object_ref_sink(factory);
	ibus_factory_add_engine(factory, "ez", IBUS_TYPE_EZ_ENGINE);
	
	IBusComponent *component;
	component = ibus_component_new("org.freedesktop.IBus.Ez",
					"Ez",
					"0.10",
					"GPL",
					"Joseph",
					"http://github/com",
					"",
					"ibus-ez");
	ibus_component_add_engine(component,
				ibus_engine_desc_new("ez",
						     "EZ",
						     "Graduation Please.",
						     "zh",
						     "GPL",
						     "Joseph",
						     "a",
						     "us"));
	;
	if(ibus_bus_register_component(bus, component)){
		cout << "succesful register" << endl;
	}else{
		cout << "Fail to register" << endl;
	}
	//ibus_bus_request_name(bus, "org.freedesktop.IBus.ez", 0);
	ibus_main();
	printf("%p\n", bus);
	cout << "In the helloWorld process" << endl;
	cout << "Here is the gtype of ez engine: " << ibus_ez_engine_get_type() << endl; 
	return 0;
}
