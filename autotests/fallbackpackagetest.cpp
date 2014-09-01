/******************************************************************************
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>                            *
*   Copyright 2014 Marco Martin <mart@kde.org>                                *
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

#include "fallbackpackagetest.h"


#include <QDebug>

#include "packagestructure.h"
#include "pluginloader.h"

void FallbackPackageTest::initTestCase()
{
    m_packagePath = QFINDTESTDATA("data/testpackage");
    m_pkg = Plasma::PluginLoader::self()->loadPackage("Plasma/Generic");
    m_pkg.setPath(m_packagePath);

    m_fallPackagePath = QFINDTESTDATA("data/testfallbackpackage");
    m_fallPkg = Plasma::PluginLoader::self()->loadPackage("Plasma/Generic");
    m_fallPkg.setPath(m_fallPackagePath);
}

void FallbackPackageTest::beforeFallback()
{
    QVERIFY(m_pkg.hasValidStructure());
    QVERIFY(m_fallPkg.hasValidStructure());

    QVERIFY(!m_pkg.filePath("ui", "otherfile.qml").isEmpty());
    QVERIFY(m_fallPkg.filePath("ui", "otherfile.qml").isEmpty());
}

void FallbackPackageTest::afterFallback()
{
    m_fallPkg.setFallbackPackage(m_pkg);

    QVERIFY(!m_fallPkg.filePath("ui", "otherfile.qml").isEmpty());
    QCOMPARE(m_pkg.filePath("ui", "otherfile.qml"), m_fallPkg.filePath("ui", "otherfile.qml"));
    QVERIFY(m_pkg.filePath("mainscript") != m_fallPkg.filePath("mainscript"));
}

void FallbackPackageTest::cycle()
{
    m_pkg.setFallbackPackage(m_fallPkg);
    m_fallPkg.setFallbackPackage(m_pkg);

    //The cycle should have been detected and filePath should take a not infinite tiume
    QCOMPARE(m_pkg.filePath("ui", "otherfile.qml"), m_fallPkg.filePath("ui", "otherfile.qml"));
}

QTEST_MAIN(FallbackPackageTest)

