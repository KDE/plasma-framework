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
    m_fallPackagePath = QFINDTESTDATA("data/testpackage");
    m_fallbackPkg = Plasma::PluginLoader::self()->loadPackage("Plasma/Generic");
    m_fallbackPkg.setPath(m_fallPackagePath);

    m_packagePath = QFINDTESTDATA("data/testfallbackpackage");
    m_pkg = Plasma::PluginLoader::self()->loadPackage("Plasma/Generic");
    m_pkg.setPath(m_packagePath);
}

void FallbackPackageTest::beforeFallback()
{
    QVERIFY(m_fallbackPkg.hasValidStructure());
    QVERIFY(m_pkg.hasValidStructure());

    //m_fallbackPkg should have otherfile.qml, m_pkg shouldn't
    QVERIFY(!m_fallbackPkg.filePath("ui", "otherfile.qml").isEmpty());
    QVERIFY(m_pkg.filePath("ui", "otherfile.qml").isEmpty());
}

void FallbackPackageTest::afterFallback()
{
    m_pkg.setFallbackPackage(m_fallbackPkg);

    //after setting the fallback, m_pkg should resolve the exact same file as m_fallbackPkg
    // for otherfile.qml
    QVERIFY(!m_pkg.filePath("ui", "otherfile.qml").isEmpty());
    QCOMPARE(m_fallbackPkg.filePath("ui", "otherfile.qml"), m_pkg.filePath("ui", "otherfile.qml"));
    QVERIFY(m_fallbackPkg.filePath("mainscript") != m_pkg.filePath("mainscript"));
}

void FallbackPackageTest::cycle()
{
    m_fallbackPkg.setFallbackPackage(m_pkg);
    m_pkg.setFallbackPackage(m_fallbackPkg);

    //The cycle should have been detected and filePath should take a not infinite time
    QTRY_COMPARE_WITH_TIMEOUT(m_fallbackPkg.filePath("ui", "otherfile.qml"), m_pkg.filePath("ui", "otherfile.qml"), 1000);
}

QTEST_MAIN(FallbackPackageTest)

