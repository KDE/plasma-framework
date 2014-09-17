/******************************************************************************
 *  Copyright 2013 Sebastian Kügler <sebas@kde.org>                           *
 *                                                                            *
 *  This library is free software; you can redistribute it and/or             *
 *  modify it under the terms of the GNU Lesser General Public                *
 *                                                                            *
 *  License as published by the Free Software Foundation; either              *
 *  version 2.1 of the License, or (at your option) version 3, or any         *
 *  later version accepted by the membership of KDE e.V. (or its              *
 *  successor approved by the membership of KDE e.V.), which shall            *
 *  act as a proxy defined in Section 6 of version 3 of the license.          *
 *                                                                            *
 *  This library is distributed in the hope that it will be useful,           *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 *  Lesser General Public License for more details.                           *
 *                                                                            *
 *  You should have received a copy of the GNU Lesser General Public          *
 *  License along with this library.  If not, see                             *
 *  <http://www.gnu.org/licenses/>.                                           *
 *                                                                            *
 ******************************************************************************/

#include "kconfigtojson.h"

#include <qcommandlineparser.h>
#include <qcommandlineoption.h>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <QTextStream>

#include <QDebug>

#include <kdesktopfile.h>
#include <kconfiggroup.h>

static QTextStream cout(stdout);
static QTextStream cerr(stderr);

KConfigToJson::KConfigToJson(QCommandLineParser *parser, const QCommandLineOption &p, const QCommandLineOption &o)
    : m_parser(parser),
      packagedir(p),
      output(o)
{
}

int KConfigToJson::runMain()
{
    if (!m_parser->isSet(packagedir)) {
        cout << "Usage --help. In short: desktoptojson -p packagedir -o outputfile.json" << endl;
        return 1;
    }

    if (!resolveFiles()) {
        cerr << "Failed to resolve filenames" << m_packageDir << m_outFile << endl;
        return 1;
    }

    if (m_packageDir.endsWith("plasma/")) {
        QDirIterator it(m_packageDir,
                        QStringList(),
                        QDir::Dirs,
                        QDirIterator::NoIteratorFlags);

        qDebug() << "package dir:" << m_packageDir;
        bool ok;
        while (it.hasNext()) {
            it.next();
            const QString _f = it.fileInfo().absoluteFilePath();
            qDebug() << "Creating cache file for " << _f;
            convertDirectory(_f, _f+"/plasma-packagecache.json");
        }
        return true;
    }
    return convertDirectory(m_packageDir, m_outFile) ? 0 : 1;
}

bool KConfigToJson::resolveFiles()
{
    if (m_parser->isSet(packagedir)) {
        m_packageDir = m_parser->value(packagedir);
        const QFileInfo fi(m_packageDir);
        if (!fi.exists()) {
            cerr << "File not found: " + m_packageDir;
            return false;
        }
        if (!fi.isAbsolute()) {
            m_packageDir = QDir::currentPath() + QDir::separator() + m_packageDir;
        }
    }
    if (!m_packageDir.endsWith(QDir::separator())) {
        m_packageDir = m_packageDir + QDir::separator();
    }

    if (m_parser->isSet(output)) {
        m_outFile = m_parser->value(output);
    } else if (!m_packageDir.isEmpty()) {
        m_outFile = m_packageDir + QStringLiteral("plasma-packagecache.json");
    }

    return m_packageDir != m_outFile && !m_packageDir.isEmpty() && !m_outFile.isEmpty();
}

bool KConfigToJson::convertDirectory(const QString& dir, const QString& dest)
{
    QVariantMap vm;
    QVariantMap pluginsVm;
    vm[QStringLiteral("Version")] = QStringLiteral("1.0");
    vm[QStringLiteral("Timestamp")] = QDateTime::currentMSecsSinceEpoch();

    QElapsedTimer t;
    t.start();
    QDirIterator it(dir,
                    QStringList(),
                    QDir::Dirs,
                    QDirIterator::NoIteratorFlags);
    qDebug() << "dest";
    qDebug() << "package dir:" << m_packageDir;
    while (it.hasNext()) {
        it.next();
        const QString _f = it.fileInfo().absoluteFilePath();
        const QString _metadata = _f+"/metadata.desktop";
        if (QFile::exists(_metadata)) {
            QVariantMap pluginMap;
            const QString pluginName = it.fileInfo().absoluteFilePath().remove(m_packageDir);
//             qDebug() << "converting: " << pluginName;
            pluginMap["Path"] = _f;
            pluginMap["PluginName"] = pluginName;
            pluginMap["MetaData"] = convert(_metadata);
            pluginsVm[pluginName] = pluginMap;
        }
        //loader.setFileName(_f);
        //const QVariantList argsWithMetaData = QVariantList() << loader.metaData().toVariantMap();

    }
    vm[QStringLiteral("Packages")] = pluginsVm;
    qDebug() << "Iterating " << m_packageDir << "took" << t.elapsed() << "msec";
    t.start();
    QJsonObject jo = QJsonObject::fromVariantMap(vm);
    QJsonDocument jdoc;
    jdoc.setObject(jo);

    QFile file(dest);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        cerr << "Failed to open " << dest << endl;
        return false;
    }

    //file.write(jdoc.toJson());
    file.write(jdoc.toBinaryData());
    cout << "Generated " << dest << endl;
    qDebug() << "Serializing and writing took " << t.elapsed() << "msec";

    return true;
}


QVariantMap KConfigToJson::convert(const QString &src)
{
    KDesktopFile df(src);
    KConfigGroup c = df.desktopGroup();

    static const QSet<QString> boolkeys = QSet<QString>()
                                          << QStringLiteral("Hidden") << QStringLiteral("X-KDE-PluginInfo-EnabledByDefault");
    static const QSet<QString> stringlistkeys = QSet<QString>()
            << QStringLiteral("X-KDE-ServiceTypes") << QStringLiteral("X-KDE-PluginInfo-Depends")
            << QStringLiteral("X-Plasma-Provides");

    QVariantMap vm;
    foreach (const QString &k, c.keyList()) {
        if (boolkeys.contains(k)) {
            vm[k] = c.readEntry(k, false);
        } else if (stringlistkeys.contains(k)) {
            vm[k] = c.readEntry(k, QStringList());
        } else {
            vm[k] = c.readEntry(k, QString());
        }
    }

    return vm;
}
