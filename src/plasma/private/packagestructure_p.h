/*
 *   Copyright © 2009 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *   Copyright 2014 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PLASMA_PACKAGESTRUCTURE_P_H
#define PLASMA_PACKAGESTRUCTURE_P_H

#include "../packagestructure.h"

#include <kpackage/package.h>
#include <kpackage/packagestructure.h>
#include <QPointer>

namespace Plasma
{

class PackageStructureWrapper : public KPackage::PackageStructure {
    Q_OBJECT

public:
    PackageStructureWrapper(Plasma::PackageStructure *structure, QObject *parent = 0, const QVariantList &args = QVariantList());

    ~PackageStructureWrapper();

    void initPackage(KPackage::Package *package) Q_DECL_OVERRIDE;

    void pathChanged(KPackage::Package *package) Q_DECL_OVERRIDE;

    KJob *install(KPackage::Package *package, const QString &archivePath, const QString &packageRoot) Q_DECL_OVERRIDE;
    KJob *uninstall(KPackage::Package *package, const QString &packageRoot) Q_DECL_OVERRIDE;

private:
    QPointer<Plasma::PackageStructure> m_struct;
    static QHash<KPackage::Package *, Plasma::Package *> s_packagesMap;
    friend class Package;
};

class PackageStructurePrivate
{
public:
    PackageStructurePrivate(PackageStructure *str)
        : internalStructure(0),
          q(str)
    {}
    ~PackageStructurePrivate()
    {}

    void installPathChanged(const QString &path);

    PackageStructure *q;
    KPackage::PackageStructure *internalStructure;
};

}

#endif
