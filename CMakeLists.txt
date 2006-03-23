kde4_header()
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### next target ###############

set(plasma_LIB_SRCS 
   plasma.cpp 
   applet.cpp 
   appletChain.cpp 
   appletLayout.cpp 
   appletCompositor.cpp )

kde4_automoc(${plasma_LIB_SRCS})

kde4_add_library(plasma SHARED ${plasma_LIB_SRCS})

target_link_libraries(plasma  ${KDE4_KDECORE_LIBS} kio )

set_target_properties(plasma PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} plasma )


########### install files ###############

install_files( /include/plasma FILES applet.h plasma.h  	appletCompositor.h appletLayout.h )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#KDE_CXXFLAGS = -UQT3_SUPPORT
#lib_LTLIBRARIES = libplasma.la
#
#libplasma_la_SOURCES = plasma.cpp applet.cpp \
#	appletChain.cpp appletLayout.cpp appletCompositor.cpp
#
#libplasma_la_METASOURCES = AUTO
#
#libplasma_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -no-undefined
#libplasma_la_LIBADD = $(LIB_KIO)
#
#libplasma_la_includedir = $(includedir)/plasma
#
#noinst_HEADERS =
#
#libplasma_la_include_HEADERS = applet.h plasma.h \
#	appletCompositor.h appletLayout.h
#
##kde_kcfg_DATA = plasmaSettings.kcfg extensionSettings.kcfg
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o $(podir)/libplasma.pot
