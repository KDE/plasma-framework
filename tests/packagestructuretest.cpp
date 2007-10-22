/******************************************************************************
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>                            *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#include "packagestructuretest.h"
#include "plasma/packages.cpp"

void PackageStructureTest::init()
{
    ps = new Plasma::PlasmoidStructure;
}

void PackageStructureTest::cleanup()
{
    delete ps;
}

void PackageStructureTest::type()
{
    QCOMPARE(ps->type(), QString("Plasmoid"));
}

#include <KDebug>
void PackageStructureTest::directories()
{
    QList<const char*> dirs;
    dirs << "images" << "config" << "configui" << "scripts";
    qSort(dirs);

    QList<const char*> psDirs = ps->directories();
    qSort(psDirs);

    QCOMPARE(psDirs, dirs);
}

void PackageStructureTest::requiredDirectories()
{
    QList<const char*> dirs;
    QCOMPARE(ps->requiredDirectories(), dirs);
}

void PackageStructureTest::files()
{
    QList<const char*> files;
    files << "mainconfiggui" << "mainconfigxml" << "mainscript";
    qSort(files);

    QList<const char*> psFiles = ps->files();
    qSort(psFiles);

    QCOMPARE(psFiles, files);
}

void PackageStructureTest::requiredFiles()
{
    QList<const char*> files;
    files << "mainscript";
    QCOMPARE(ps->requiredFiles(), files);
}

void PackageStructureTest::path()
{
    QCOMPARE(ps->path("images"), QString("images"));
    QCOMPARE(ps->path("mainscript"), QString("code/main"));
}

void PackageStructureTest::name()
{
    QCOMPARE(ps->name("config"), i18n("Configuration Definitions"));
    QCOMPARE(ps->name("mainscript"), i18n("Main Script File"));
}

void PackageStructureTest::required()
{
    QVERIFY(ps->required("mainscript"));
}

void PackageStructureTest::mimetypes()
{
    QStringList mimetypes;
    mimetypes << "image/svg+xml" << "image/png" << "image/jpeg";
    QCOMPARE(ps->mimetypes("images"), mimetypes);
}

QTEST_KDEMAIN(PackageStructureTest, NoGUI)

//#include "packagestructuretest.moc"

