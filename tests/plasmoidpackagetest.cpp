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
#include "../config-plasma.h"

#include <QDir>
#include <QFile>
#include <kzip.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>

#include "plasma/applet.h"

void PlasmoidPackageTest::init()
{
    m_package = QString("Package");
    m_packageRoot = QDir::homePath() + "/.kde-unit-test/packageRoot";
    m_defaultPackage = Plasma::Package::load("Plasma/Applet");
    cleanup(); // to prevent previous runs from interfering with this one
}

void PlasmoidPackageTest::cleanup()
{
    qDebug() << "CLEANEAING UP!";
    // Clean things up.
    KIO::NetAccess::del(KUrl(QDir::homePath() + QLatin1String("/.kde-unit-test/packageRoot")), 0);
}

void PlasmoidPackageTest::createTestPackage(const QString &packageName)
{
    QDir pRoot(m_packageRoot);
    // Create the root and package dir.
    if (!pRoot.exists()) {
        QVERIFY(QDir().mkpath(m_packageRoot));
    }

    // Create the package dir
    QVERIFY(QDir().mkpath(m_packageRoot + "/" + packageName));

    // Create the metadata.desktop file
    QFile file(m_packageRoot + "/" + packageName + "/metadata.desktop");

    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

    QTextStream out(&file);
    out << "[Desktop Entry]\n";
    out << "Name=" << packageName << "\n";
    out << "X-KDE-PluginInfo-Name=" << packageName << "\n";
    file.flush();
    file.close();

    // Create the code dir.
    QVERIFY(QDir().mkpath(m_packageRoot + "/" + packageName + "/contents/code"));

    // Create the main file.
    file.setFileName(m_packageRoot + "/" + packageName + "/contents/code/main");
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

    out << "THIS IS A PLASMOID SCRIPT.....";
    file.flush();
    file.close();

    // Now we have a minimal plasmoid package which is valid. Let's add some
    // files to it for test purposes.

    // Create the images dir.
    QVERIFY(QDir().mkpath(m_packageRoot + "/" + packageName + "/contents/images"));
    file.setFileName(m_packageRoot + "/" + packageName + "/contents/images/image-1.svg");

    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

    out << "<svg>This is a test image</svg>";
    file.flush();
    file.close();

    file.setFileName(m_packageRoot + "/" + packageName + "/contents/images/image-2.svg");

    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

    out.setDevice(&file);
    out << "<svg>This is another test image</svg>";
    file.flush();
    file.close();
    qDebug() << "SUUUUUUUUCCCCCCCCESSSSSSSSS";
}

void PlasmoidPackageTest::isValid()
{
    Plasma::Package *p = new Plasma::Package(m_defaultPackage);
    p->setPath(m_packageRoot + '/' + m_package);
    kDebug() << "package path is" << p->path();

    // A PlasmoidPackage is valid when:
    // - The package root exists.
    // - The package root consists an file named "code/main"
    QVERIFY(!p->isValid());

    // Create the root and package dir.
    QVERIFY(QDir().mkpath(m_packageRoot));
    QVERIFY(QDir().mkpath(m_packageRoot + "/" + m_package));

    // Should still be invalid.
    delete p;
    p = new Plasma::Package(m_defaultPackage);
    p->setPath(m_packageRoot + '/' + m_package);
    QVERIFY(!p->isValid());

    // Create the metadata.desktop file.
    QFile file(m_packageRoot + "/" + m_package + "/metadata.desktop");
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

    QTextStream out(&file);
    out << "[Desktop Entry]\n";
    out << "Name=test\n";
    out << "Description=Just a test desktop file";
    file.flush();
    file.close();

    // Create the code dir.
    QVERIFY(QDir().mkpath(m_packageRoot + "/" + m_package + "/contents/code"));

    // No main file yet so should still be invalid.
    delete p;
    p = new Plasma::Package(m_defaultPackage);
    p->setPath(m_packageRoot + '/' + m_package);
    QVERIFY(!p->isValid());

    // Create the main file.
    file.setFileName(m_packageRoot + "/" + m_package + "/contents/code/main");
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

    out.setDevice(&file);
    out << "THIS IS A PLASMOID SCRIPT.....\n";
    file.flush();
    file.close();

    file.setPermissions(QFile::ReadUser | QFile::WriteUser);
    // Main file exists so should be valid now.
    delete p;
    p = new Plasma::Package(m_defaultPackage);
    p->setPath(m_packageRoot + '/' + m_package);
    QVERIFY(p->isValid());
    QCOMPARE(p->contentsHash(), QString("db0b38c2b4fe21a9f37923cc25152340de055f6d"));
    delete p;
}

void PlasmoidPackageTest::filePath()
{
    // Package::filePath() returns
    // - {package_root}/{package_name}/path/to/file if the file exists
    // - QString() otherwise.
    Plasma::Package *p = new Plasma::Package(m_defaultPackage);
    p->setPath(m_packageRoot + '/' + m_package);

    QCOMPARE(p->filePath("scripts", "main"), QString());

    QVERIFY(QDir().mkpath(m_packageRoot + "/" + m_package + "/contents/code"));
    QFile file(m_packageRoot + "/" + m_package + "/contents/code/main");
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

    QTextStream out(&file);
    out << "THIS IS A PLASMOID SCRIPT.....";
    file.flush();
    file.close();

    // The package is valid by now so a path for code/main should get returned.
    delete p;
    p = new Plasma::Package(m_defaultPackage);
    p->setPath(m_packageRoot + '/' + m_package);

    const QString path = KStandardDirs::realFilePath(m_packageRoot + "/" + m_package + "/contents/code/main");

    // Two ways to get the same info.
    // 1. Give the file type which refers to a class of files (a directory) in
    //    the package structure and the file name.
    // 2. Give the file type which refers to a file in the package structure.
    //
    // NOTE: scripts, main and mainscript are defined in packages.cpp and are
    //       specific for a PlasmoidPackage.
    QCOMPARE(p->filePath("scripts", "main"), path);
    QCOMPARE(p->filePath("mainscript"), path);
    delete p;
}

void PlasmoidPackageTest::entryList()
{
    // Create a package named @p packageName which is valid and has some images.
    createTestPackage("SomePlasmoid");

    // Create a package object and verify that it is valid.
    Plasma::Package *p = new Plasma::Package(m_defaultPackage);
    p->setPath(m_packageRoot + '/' + m_package);
    QVERIFY(p->isValid());

    // Now we have a valid package that should contain the following files in
    // given filetypes:
    // fileTye - Files
    // scripts - {"main"}
    // images - {"image-1.svg", "image-2.svg"}
    QStringList files = p->entryList("scripts");
    QCOMPARE(files.size(), 1);
    QVERIFY(files.contains("main"));

    files = p->entryList("images");
    QCOMPARE(files.size(), 2);
    QVERIFY(files.contains("image-1.svg"));
    QVERIFY(files.contains("image-2.svg"));
    delete p;
}

void PlasmoidPackageTest::createAndInstallPackage()
{
    createTestPackage("plasmoid_to_package");
    const QString packagePath = m_packageRoot + '/' + "testpackage.plasmoid";

    KZip creator(packagePath);
    QVERIFY(creator.open(QIODevice::WriteOnly));
    creator.addLocalDirectory(m_packageRoot + '/' + "plasmoid_to_package", ".");
    creator.close();
    KIO::NetAccess::del(KUrl(m_packageRoot + "/plasmoid_to_package"), 0);

    QVERIFY(QFile::exists(packagePath));

    KZip package(packagePath);
    QVERIFY(package.open(QIODevice::ReadOnly));
    const KArchiveDirectory *dir = package.directory();
    QVERIFY(dir);
    QVERIFY(dir->entry("metadata.desktop"));
    const KArchiveEntry *contentsEntry = dir->entry("contents");
    QVERIFY(contentsEntry);
    QVERIFY(contentsEntry->isDirectory());
    const KArchiveDirectory *contents = static_cast<const KArchiveDirectory *>(contentsEntry);
    QVERIFY(contents->entry("code"));
    QVERIFY(contents->entry("images"));

    Plasma::Package *p = new Plasma::Package(m_defaultPackage);
    QVERIFY(p->installPackage(packagePath, m_packageRoot));
    const QString installedPackage = m_packageRoot + "/plasmoid_to_package";

    QVERIFY(QFile::exists(installedPackage));

    p->setPath(installedPackage);
    QVERIFY(p->isValid());
    delete p;
}

QTEST_KDEMAIN(PlasmoidPackageTest, NoGUI)
