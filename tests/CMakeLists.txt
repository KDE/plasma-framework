include_directories( ${CMAKE_SOURCE_DIR}/workspace/plasma/lib   )


########### next target ###############

set(testAppletInfo_SRCS testAppletInfo.cpp )

kde4_automoc(${testAppletInfo_SRCS})

kde4_add_executable(testAppletInfo ${testAppletInfo_SRCS})

target_link_libraries(testAppletInfo  ${KDE4_KDECORE_LIBS} )


########### install files ###############





#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/workspace/plasma/lib $(all_includes)
#
#AM_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(KDE_RPATH)
#
#noinst_PROGRAMS = testAppletInfo
#noinst_HEADERS = testAppletInfo.h
#
#METASOURCES = AUTO
#
#LDADD = ../libplasma.la -lQtTest_debug
#testAppletInfo_SOURCES = testAppletInfo.cpp
#
#check-local:
#	opwd=$PWD; cd $(srcdir) && $$opwd/testAppletInfo
#
