#AM_CXXFLAGS = -E
libexec_PROGRAMS = ibus-engine-ez
ibus_engine_ez_SOURCES = hello.cpp engine.cpp
ibus_engine_ez_CPPFLAGS = \
	@IBUS_CFLAGS@ 
ibus_engine_ez_LDADD = \
	@IBUS_LIBS@
component_DATA = \
	ez.xml \
	$(NULL)
componentdir = /usr/share/ibus/component

ez.xml : ez.xml.in
	$(AM_V_GEN) \
	( \
		s=`cat $<`; \
		eval "echo \"$${s}\""; \
	) > $@
CLEANFILES = \
	ez.xml \
	$(NULL)
	
