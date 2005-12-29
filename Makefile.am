INCLUDES = $(all_includes)
KDE_CXXFLAGS = -UQT3_SUPPORT
lib_LTLIBRARIES = libplasma.la

libplasma_la_SOURCES = appletinfo.cpp plasma.cpp applet.cpp appletChain.cpp

libplasma_la_METASOURCES = AUTO

libplasma_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -no-undefined
libplasma_la_LIBADD = $(LIB_KIO)

libplasma_la_includedir = $(includedir)/plasma

noinst_HEADERS =

libplasma_la_include_HEADERS = appletinfo.h applet.h plasma.h

#kde_kcfg_DATA = plasmaSettings.kcfg extensionSettings.kcfg

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/libplasma.pot
