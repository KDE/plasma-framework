/*
    Copyright (C) 2019 Aleix Pol <apol@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QString>
#include <QDir>
#include <QStandardPaths>
#include <QTest>

namespace Plasma {
namespace TestUtils {

static void copyPath(const QString &src, const QString &dst) {
    QDir dir(src);
    Q_ASSERT(dir.exists());

    foreach (const auto &d, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString dst_path = dst + QLatin1Char('/') + d;
        dir.mkpath(dst_path);
        copyPath(src + QLatin1Char('/') + d, dst_path);
    }

    foreach (const auto &f, dir.entryList(QDir::Files)) {
        QFile::copy(src + QLatin1Char('/') + f, dst + QLatin1Char('/') + f);
    }
}

static void installPlasmaTheme() {
    QStandardPaths::setTestModeEnabled(true);
    const auto qttestPath = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).constFirst();
    Q_ASSERT(!qttestPath.isEmpty());
    QDir themePath(qttestPath + QLatin1String("/plasma/desktoptheme/default"));

    auto data = QFINDTESTDATA("../src/desktoptheme/breeze/metadata.desktop");
    QFileInfo f(data);
    QVERIFY(f.dir().mkpath(themePath.path()));

    copyPath(f.dir().filePath("default.gzipped"), themePath.path());
    QFile::copy(f.dir().filePath("metadata.desktop"), themePath.filePath("metadata.desktop"));
}

} //TestUtils
} //Plasma
