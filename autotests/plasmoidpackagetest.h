/*
    SPDX-FileCopyrightText: 2007 Bertjan Broeksema <b.broeksema@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PACKAGETEST_H

#include <QTest>

#include "plasma/package.h"
#include "plasma/packagestructure.h"

class PlasmoidPackageTest : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

private Q_SLOTS:
    void createAndInstallPackage();
    void isValid();
    void filePath();
    void entryList();

    void packageInstalled(KJob *j);
    void packageUninstalled(KJob *j);

private:
    void createTestPackage(const QString &packageName);

    QString m_packageRoot;
    QString m_package;
    KJob *m_packageJob;
    Plasma::Package m_defaultPackage;
    Plasma::PackageStructure *m_defaultPackageStructure;
};

#endif
