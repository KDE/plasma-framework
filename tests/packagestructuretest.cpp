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

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>

#include "applet.h"
#include "pluginloader.h"

class NoPrefixes : public Plasma::Package
{
public:
    explicit NoPrefixes()
        : Plasma::Package()
    {
        setContentsPrefixPaths(QStringList());
        addDirectoryDefinition("bin", "bin", "bin");
        setPath("/");
    }
};


void PackageStructureTest::init()
{
    m_packagePath = QString::fromLatin1(KDESRCDIR) + "signedPackage/";
    ps = Plasma::PluginLoader::self()->loadPackage("Plasma/Applet");
    ps.setPath(m_packagePath);
}

void PackageStructureTest::emptyContentsPrefix()
{
    NoPrefixes package;
    QString path(package.filePath("bin", "ls"));
    //qDebug() << path;
    QCOMPARE(path, QString("/bin/ls"));
}

void PackageStructureTest::directories()
{
    QList<const char*> dirs;
    dirs << "animations" << "config" << "data" << "images" << "scripts" << "translations" << "ui";

    QList<const char*> psDirs = ps.directories();

    QCOMPARE(dirs.count(), psDirs.count());

    for (int i = 0; i < psDirs.count(); ++i) {
        QVERIFY(qstrcmp(dirs[i], psDirs[i]) == 0);
    }
}

void PackageStructureTest::requiredDirectories()
{
    QList<const char*> dirs;
    QCOMPARE(ps.requiredDirectories(), dirs);
}

void PackageStructureTest::files()
{
    QList<const char*> files;
    files << "defaultconfig" << "mainconfigui" << "mainconfigxml" << "mainscript";

    QList<const char*> psFiles = ps.files();

    //for (int i = 0; i < psFiles.count(); ++i) {
    //    qDebug() << psFiles[i];
    //}
    QCOMPARE(files.count(), psFiles.count());
    for (int i = 0; i < files.count(); ++i) {
        QCOMPARE(files[i], psFiles[i]);
    }
}

void PackageStructureTest::requiredFiles()
{
    QList<const char*> files;
    files << "mainscript";

    QList<const char*> psFiles = ps.requiredFiles();

    QCOMPARE(files.count(), psFiles.count());
    for (int i = 0; i < files.count(); ++i) {
        QCOMPARE(files[i], psFiles[i]);
    }
}

void PackageStructureTest::path()
{
    qDebug() << "real paths are" << ps.filePath("images") << ps.filePath("mainscript");
    qDebug() << "we wants" << QString(m_packagePath + QString("contents/images")) << QString(m_packagePath + QString("contents/code/main.js"));
    QCOMPARE(ps.filePath("images"), QString(m_packagePath + QString("contents/images")));
    QCOMPARE(ps.filePath("mainscript"), QString(m_packagePath + QString("contents/code/main.js")));
}

void PackageStructureTest::name()
{
    QCOMPARE(ps.name("config"), i18n("Configuration Definitions"));
    QCOMPARE(ps.name("mainscript"), i18n("Main Script File"));
}

void PackageStructureTest::required()
{
    QVERIFY(ps.isRequired("mainscript"));
}

void PackageStructureTest::mimeTypes()
{
    QStringList mimeTypes;
    mimeTypes << "image/svg+xml" << "image/png" << "image/jpeg";
    QCOMPARE(ps.mimeTypes("images"), mimeTypes);
}

QTEST_KDEMAIN(PackageStructureTest, NoGUI)

//#include "packagestructuretest.moc"

