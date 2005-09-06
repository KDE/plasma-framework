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

#include "testAppletInfo.h"
#include "appletInfo.h"

TestAppletInfo::TestAppletInfo(QObject* parent)
    : QObject(parent)
{
    QString pwd = QDir::currentPath();
    notUniqueNative = new Plasma::AppletInfo(pwd + "/nativeApplet.desktop");
    uniqueJavascript = new Plasma::AppletInfo(pwd + "/uniqueJavaScriptApplet.desktop");
}

void TestAppletInfo::name_data(QtTestTable &t)
{
    t.defineElement("QString", "expected");
    t.defineElement("QString", "actual");

    *t.newData("Non-Unique Native Applet") << t.name()
                                           << "Non-Unique Native Applet";
    *t.newData("Unique Javascript Applet") << t.name()
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

    *t.newData("Non-Unique Native Applet") << t.comment()
                                           << "A natively compiled applet";
    *t.newData("Unique Javascript Applet") << t.comment()
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

    *t.newData("Non-Unique Native Applet") << t.icon()
                                           << "native";
    *t.newData("Unique Javascript Applet") << t.icon()
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

    *t.newData("Non-Unique Native Applet") << t.library()
                                           << "plasma_applet_native";
    *t.newData("Unique Javascript Applet") << t.library()
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

    *t.newData("Non-Unique Native Applet") << t.languageBindings()
                                           << "native";
    *t.newData("Unique Javascript Applet") << t.languageBindings()
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

    *t.newData("Non-Unique Native Applet") << t.desktopFilePath()
                                           << pwd + "/nativeApplet.desktop";
    *t.newData("Unique Javascript Applet") << t.desktopFilePath()
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

    *t.newData("Non-Unique Native Applet") << t.desktopFile()
                                           << "nativeApplet.desktop";
    *t.newData("Unique Javascript Applet") << t.desktopFile()
                                           << "uniqueJavaScriptApplet.desktop";
}

void TestAppletInfo::desktopFile()
{
    FETCH(QString, expected);
    FETCH(QString, actual);

    COMPARE(expected, actual);
}

void TestAppletInfo::desktopFile_data(QtTestTable &t)
{
    t.defineElement("QString", "expected");
    t.defineElement("QString", "actual");

    *t.newData("Non-Unique Native Applet") << t.unique()
                                           << "nativeApplet.desktop";
    *t.newData("Unique Javascript Applet") << t.unique()
                                           << "uniqueJavaScriptApplet.desktop";
}

void TestAppletInfo::unique()
{
    FETCH(QString, expected);
    FETCH(QString, actual);

    COMPARE(expected, actual);
}

void TestAppletInfo::hidden_data(QtTestTable &t)
{
    t.defineElement("QString", "expected");
    t.defineElement("QString", "actual");

    *t.newData("Non-Unique Native Applet") << t.unique()
                                           << "nativeApplet.desktop";
    *t.newData("Unique Javascript Applet") << t.unique()
                                           << "uniqueJavaScriptApplet.desktop";
}

void TestAppletInfo::hidden()
{
    FETCH(QString, expected);
    FETCH(QString, actual);

    COMPARE(expected, actual);
}

QTTEST_MAIN(TestQString)
#include "testAppletInfo.moc"
