INCLUDES = -I$(top_srcdir)/workspace/plasma/lib $(all_includes)

AM_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(KDE_RPATH)

noinst_PROGRAMS = testAppletInfo
noinst_HEADERS = testAppletInfo.h

METASOURCES = AUTO

LDADD = ../libplasma.la -lQtTest_debug
testAppletInfo_SOURCES = testAppletInfo.cpp

check:
	opwd=$PWD; cd $(srcdir) && $$opwd/testAppletInfo

