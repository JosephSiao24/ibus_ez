#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <ibus.h>
#include <iostream>
using namespace std;
#define PAGESIZE 9
#define IBUS_TYPE_EZ_ENGINE \ 
	(ibus_ez_engine_get_type())
GType ibus_ez_engine_get_type (void);

#endif
