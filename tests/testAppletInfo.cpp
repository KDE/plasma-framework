/*
Unit tests for Plasma::AppletInfo

Copyright (C) 2005 Aaron Seigo <aseigo@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2 as published by
the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <QDir>

#include <kaboutdata.h>
#include <kinstance.h>

#include "appletinfo.h"
#include "testAppletInfo.h"

TestAppletInfo::TestAppletInfo(QObject* parent)
    : QObject(parent)
{
    m_aboutData = new KAboutData("Test Applet Info", "testappletinfo", "1");
    m_instance = new KInstance("testappletinfo");
    QString pwd = QDir::currentPath();
    notUniqueNative = new Plasma::AppletInfo(pwd + "/nativeApplet.desktop");
    uniqueJavascript = new Plasma::AppletInfo(pwd + "/uniqueJavaScriptApplet.desktop");
}

TestAppletInfo::~TestAppletInfo()
{
    delete m_instance;
    delete m_aboutData;
}

void TestAppletInfo::name_data(QtTestTable &t)
{
    t.defineElement("QString", "expected");
    t.defineElement("QString", "actual");

    *t.newData("Non-Unique Native Applet") << notUniqueNative->name()
                                           << "Non-Unique Native Applet";
    *t.newData("Unique Javascript Applet") << uniqueJavascript->name()
                                           << "Unique Javascript Applet";
}

void TestAppletInfo::name()
{
    FETCH(QString, expected);
    FETCH(QString, actual);

    COMPARE(expected, actual);
}

void TestAppletInfo::comment_data(QtTestTable &t)
{
    t.defineElement("QString", "expected");
    t.defineElement("QString", "actual");

    *t.newData("Non-Unique Native Applet") << notUniqueNative->comment()
                                           << "A natively compiled applet";
    *t.newData("Unique Javascript Applet") << uniqueJavascript->comment()
                                           << "An applet written in JavaScript";
}

void TestAppletInfo::comment()
{
    FETCH(QString, expected);
    FETCH(QString, actual);

    COMPARE(expected, actual);
}

void TestAppletInfo::icon_data(QtTestTable &t)
{
    t.defineElement("QString", "expected");
    t.defineElement("QString", "actual");

    *t.newData("Non-Unique Native Applet") << notUniqueNative->icon()
                                           << "native";
    *t.newData("Unique Javascript Applet") << uniqueJavascript->icon()
                                           << "javascript";
}

void TestAppletInfo::icon()
{
    FETCH(QString, expected);
    FETCH(QString, actual);

    COMPARE(expected, actual);
}

void TestAppletInfo::library_data(QtTestTable &t)
{
    t.defineElement("QString", "expected");
    t.defineElement("QString", "actual");

    *t.newData("Non-Unique Native Applet") << notUniqueNative->library()
                                           << "plasma_applet_native";
    *t.newData("Unique Javascript Applet") << uniqueJavascript->library()
                                           << "plasma_applet_javascript";
}

void TestAppletInfo::library()
{
    FETCH(QString, expected);
    FETCH(QString, actual);

    COMPARE(expected, actual);
}

void TestAppletInfo::languageBindings_data(QtTestTable &t)
{
    t.defineElement("QString", "expected");
    t.defineElement("QString", "actual");

    *t.newData("Non-Unique Native Applet") << notUniqueNative->languageBindings()
                                           << "native";
    *t.newData("Unique Javascript Applet") << uniqueJavascript->languageBindings()
                                           << "javascript";
}

void TestAppletInfo::languageBindings()
{
    FETCH(QString, expected);
    FETCH(QString, actual);

    COMPARE(expected, actual);
}

void TestAppletInfo::desktopFilePath_data(QtTestTable &t)
{
    t.defineElement("QString", "expected");
    t.defineElement("QString", "actual");

    QString pwd = QDir::currentPath();

    *t.newData("Non-Unique Native Applet") << notUniqueNative->desktopFilePath()
                                           << pwd + "/nativeApplet.desktop";
    *t.newData("Unique Javascript Applet") << uniqueJavascript->desktopFilePath()
                                           << pwd + "/uniqueJavaScriptApplet.desktop";
}

void TestAppletInfo::desktopFilePath()
{
    FETCH(QString, expected);
    FETCH(QString, actual);

    COMPARE(expected, actual);
}

void TestAppletInfo::desktopFile_data(QtTestTable &t)
{
    t.defineElement("QString", "expected");
    t.defineElement("QString", "actual");

    *t.newData("Non-Unique Native Applet") << notUniqueNative->desktopFile()
                                           << "nativeApplet.desktop";
    *t.newData("Unique Javascript Applet") << uniqueJavascript->desktopFile()
                                           << "uniqueJavaScriptApplet.desktop";
}

void TestAppletInfo::desktopFile()
{
    FETCH(QString, expected);
    FETCH(QString, actual);

    COMPARE(expected, actual);
}

void TestAppletInfo::unique_data(QtTestTable &t)
{
    t.defineElement("bool", "expected");
    t.defineElement("bool", "actual");

    *t.newData("Non-Unique Native Applet") << notUniqueNative->unique()
                                           << false;
    *t.newData("Unique Javascript Applet") << uniqueJavascript->unique()
                                           << true;
}

void TestAppletInfo::unique()
{
    FETCH(bool, expected);
    FETCH(bool, actual);

    COMPARE(expected, actual);
}

void TestAppletInfo::hidden_data(QtTestTable &t)
{
    t.defineElement("bool", "expected");
    t.defineElement("bool", "actual");

    *t.newData("Non-Unique Native Applet") << notUniqueNative->hidden()
                                           << false;
    *t.newData("Unique Javascript Applet") << uniqueJavascript->hidden()
                                           << true;
}

void TestAppletInfo::hidden()
{
    FETCH(bool, expected);
    FETCH(bool, actual);

    COMPARE(expected, actual);
}

void TestAppletInfo::assignment()
{
    Plasma::AppletInfo assigned = *notUniqueNative;
    COMPARE(assigned.name(), notUniqueNative->name());
    COMPARE(assigned.comment(), notUniqueNative->comment());
    COMPARE(assigned.icon(), notUniqueNative->icon());
    COMPARE(assigned.library(), notUniqueNative->library());
    COMPARE(assigned.languageBindings(), notUniqueNative->languageBindings());
    COMPARE(assigned.desktopFilePath(), notUniqueNative->desktopFilePath());
    COMPARE(assigned.desktopFile(), notUniqueNative->desktopFile());
    COMPARE(assigned.unique(), notUniqueNative->unique());
    COMPARE(assigned.hidden(), notUniqueNative->hidden());
}

void TestAppletInfo::copyConstructor()
{
    Plasma::AppletInfo* tempCopy = new Plasma::AppletInfo(*notUniqueNative);
    Plasma::AppletInfo copied(*tempCopy);
    delete tempCopy;
    COMPARE(copied.name(), notUniqueNative->name());
    COMPARE(copied.comment(), notUniqueNative->comment());
    COMPARE(copied.icon(), notUniqueNative->icon());
    COMPARE(copied.library(), notUniqueNative->library());
    COMPARE(copied.languageBindings(), notUniqueNative->languageBindings());
    COMPARE(copied.desktopFilePath(), notUniqueNative->desktopFilePath());
    COMPARE(copied.desktopFile(), notUniqueNative->desktopFile());
    COMPARE(copied.unique(), notUniqueNative->unique());
    COMPARE(copied.hidden(), notUniqueNative->hidden());
}

QTTEST_MAIN(TestAppletInfo)
#include "testAppletInfo.moc"
