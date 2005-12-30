INCLUDES = $(all_includes)
KDE_CXXFLAGS = -UQT3_SUPPORT
lib_LTLIBRARIES = libplasma.la

libplasma_la_SOURCES = plasma.cpp applet.cpp \
	appletChain.cpp appletLayout.cpp appletCompositor.cpp

libplasma_la_METASOURCES = AUTO

libplasma_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -no-undefined
libplasma_la_LIBADD = $(LIB_KIO)

libplasma_la_includedir = $(includedir)/plasma

noinst_HEADERS =

libplasma_la_include_HEADERS = applet.h plasma.h \
	appletCompositor.h appletLayout.h

#kde_kcfg_DATA = plasmaSettings.kcfg extensionSettings.kcfg

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/libplasma.pot
