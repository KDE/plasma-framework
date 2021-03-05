/*
    SPDX-FileCopyrightText: 2009 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_PACKAGE_P_H
#define PLASMA_PACKAGE_P_H

#include "../package.h"

#include <QExplicitlySharedDataPointer>

#include <kpackage/package.h>

namespace Plasma
{
class PackagePrivate : public QSharedData
{
public:
    PackagePrivate();
    ~PackagePrivate();

    void installFinished(KJob *job);
    void uninstallFinished(KJob *job);

    QString servicePrefix;
    KPackage::Package *internalPackage;
    Package *fallbackPackage;
    PackageStructure *structure;
};

}

#endif
