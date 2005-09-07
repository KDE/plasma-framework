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

#ifndef testappletinfo_h_
#define testappletinfo_h_

#include <QtTest/QtTest>

namespace Plasma
{
    class AppletInfo;
}
class KAboutData;
class KInstance;

class TestAppletInfo: public QObject
{
    Q_OBJECT
    public:
        TestAppletInfo(QObject* parent = 0);

    private slots:
        void name_data(QtTestTable& t);
        void name();
        void comment_data(QtTestTable& t);
        void comment();
        void icon_data(QtTestTable& t);
        void icon();
        void library_data(QtTestTable& t);
        void library();
        void languageBindings_data(QtTestTable& t);
        void languageBindings();
        void desktopFilePath_data(QtTestTable& t);
        void desktopFilePath();
        void desktopFile_data(QtTestTable& t);
        void desktopFile();
        void unique_data(QtTestTable& t);
        void unique();
        void hidden_data(QtTestTable& t);
        void hidden();

    private:
        Plasma::AppletInfo* notUniqueNative;
        Plasma::AppletInfo* uniqueJavascript;
        KAboutData* m_aboutData;
        KInstance* m_instance;
};

#endif
