/******************************************************************************
*   Copyright 2007 by Bertjan Broeksema <b.broeksema@kdemail.net>             *
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

#include "plasmoidpackagetest.h"

#include <QDir>
#include <QFile>

#include "plasma/packages.cpp"

void PlasmoidPackageTest::init()
{
    mPackage = QString("Package");
    mPackageRoot = QDir::homePath() + "/.kde-unit-test/packageRoot";
    
    ps = new Plasma::PlasmoidStructure;
    p = new Plasma::Package(mPackageRoot, mPackage, *ps);
}

void PlasmoidPackageTest::cleanup()
{
    delete ps;
    delete p;
    
    // Clean things up.
    removeDir(QLatin1String("packageRoot/" + mPackage.toLatin1() + "/code"));
    removeDir(QLatin1String("packageRoot/" + mPackage.toLatin1()));
    QDir().rmpath(QDir::homePath() + "/.kde-unit-test/packageRoot");
}

// Copied from ktimezonetest.h
void PlasmoidPackageTest::removeDir(const QString &subdir)
{
    QDir local = QDir::homePath() + QLatin1String("/.kde-unit-test/") + subdir;
    foreach(const QString &file, local.entryList(QDir::Files))
        if(!local.remove(file))
            qWarning("%s: removing failed", qPrintable( file ));
    QCOMPARE((int)local.entryList(QDir::Files).count(), 0);
    local.cdUp();
    QString subd = subdir;
    subd.remove(QRegExp("^.*/"));
    local.rmpath(subd);
}

void PlasmoidPackageTest::isValid()
{
    // A PlasmoidPackage is valid when:
    // - The package root exists.
    // - The package root consists an file named "code/main"
    QVERIFY(!p->isValid());
    
    // Create the root and package dir.
    QVERIFY(QDir().mkpath(mPackageRoot));
    QVERIFY(QDir().mkpath(mPackageRoot + "/" + mPackage));
    
    // Should still be invalid.
    delete p;
    p = new Plasma::Package(mPackageRoot, mPackage, *ps);
    qDebug() << p->isValid();
    QVERIFY(!p->isValid());
    
    // Create the code dir.
    QVERIFY(QDir().mkpath(mPackageRoot + "/" + mPackage + "/code"));
    
    // No main file yet so should still be invalid.
    delete p;
    p = new Plasma::Package(mPackageRoot, mPackage, *ps);
    QVERIFY(!p->isValid());
    
    // Create the main file.
    QFile file(mPackageRoot + "/" + mPackage + "/code/main");
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    
    QTextStream out(&file);
    out << "THIS IS A PLASMOID SCRIPT.....";
    file.flush();
    file.close();
    
    // Main file exists so should be valid now.
    delete p;
    p = new Plasma::Package(mPackageRoot, mPackage, *ps);
    QVERIFY(p->isValid());
}

QTEST_KDEMAIN(PlasmoidPackageTest, NoGUI)
