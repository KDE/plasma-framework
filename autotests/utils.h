/*
    SPDX-FileCopyrightText: 2019 Aleix Pol <apol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef UTILS_H
#define UTILS_H

#include <QDir>
#include <QStandardPaths>
#include <QString>
#include <QTest>

namespace Plasma
{
namespace TestUtils
{
static void copyPath(const QString &src, const QString &dst)
{
    QDir dir(src);

    const auto dirList = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const auto &d : dirList) {
        QString dst_path = dst + QLatin1Char('/') + d;
        dir.mkpath(dst_path);
        copyPath(src + QLatin1Char('/') + d, dst_path);
    }

    const auto entryList = dir.entryList(QDir::Files);
    for (const auto &f : entryList) {
        QFile::copy(src + QLatin1Char('/') + f, dst + QLatin1Char('/') + f);
    }
}

static void installPlasmaTheme(const QString &theme = QStringLiteral("breeze"))
{
    QString destinationTheme = (theme == QLatin1String("breeze") ? QStringLiteral("default") : theme);

    QStandardPaths::setTestModeEnabled(true);
    const auto qttestPath = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).constFirst();
    Q_ASSERT(!qttestPath.isEmpty());
    QDir themePath(qttestPath + QLatin1String("/plasma/desktoptheme/") + destinationTheme);

    auto data = QFINDTESTDATA("../src/desktoptheme/" + theme + "/metadata.desktop");
    QFileInfo f(data);
    QVERIFY(f.dir().mkpath(themePath.path()));

    copyPath(f.dir().filePath("default.gzipped"), themePath.path());
    QFile::copy(f.dir().filePath("metadata.desktop"), themePath.filePath("metadata.desktop"));

    const QString colorsFile = QFINDTESTDATA("../src/desktoptheme/" + theme + "/colors");
    if (!colorsFile.isEmpty()) {
        QFile::copy(colorsFile, themePath.filePath("colors"));
    }
}

} // TestUtils
} // Plasma

#endif
