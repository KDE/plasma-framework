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

#include <QtTest>

class Plasma::AppletInfo;

class TestAppletInfo: public QObject
{
    Q_OBJECT
    public:
        TestAppletInfo(QObject* parent = 0);

    private slots:
        void name_data(QTestTable& t);
        void name();
        void comment_data(QTestTable& t);
        void comment();
        void icon_data(QTestTable& t);
        void icon();
        void library_data(QTestTable& t);
        void library();
        void languageBindings_data(QTestTable& t);
        void languageBindings();
        void desktopFilePath_data(QTestTable& t);
        void desktopFilePath();
        void desktopFile_data(QTestTable& t);
        void desktopFile();
        void unique_data(QTestTable& t);
        void unique();
        void hidden_data(QTestTable& t);
        void hidden();

    private:
        Plasma::AppletInfo* notUniqueNative;
        Plasma::AppletInfo* uniqueJavascript;
};
