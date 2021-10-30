/*
    SPDX-FileCopyrightText: 2009 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_PACKAGESTRUCTURE_P_H
#define PLASMA_PACKAGESTRUCTURE_P_H

#include "../packagestructure.h"

#include <QPointer>
#include <kpackage/package.h>
#include <kpackage/packagestructure.h>

namespace Plasma
{
class PackageStructureWrapper : public KPackage::PackageStructure
{
    Q_OBJECT

public:
    PackageStructureWrapper(Plasma::PackageStructure *structure, QObject *parent = nullptr, const QVariantList &args = QVariantList());

    ~PackageStructureWrapper() override;

    void initPackage(KPackage::Package *package) override;

    void pathChanged(KPackage::Package *package) override;

    KJob *install(KPackage::Package *package, const QString &archivePath, const QString &packageRoot) override;
    KJob *uninstall(KPackage::Package *package, const QString &packageRoot) override;

private:
    QPointer<Plasma::PackageStructure> m_struct;
    static QHash<KPackage::Package *, Plasma::Package *> s_packagesMap;
    friend class Package;
};

class PackageStructurePrivate
{
public:
    PackageStructurePrivate(PackageStructure *str)
        : q(str)
        , internalStructure(nullptr)
    {
    }
    ~PackageStructurePrivate()
    {
    }

    void installPathChanged(const QString &path);

    PackageStructure *q;
    KPackage::PackageStructure *internalStructure;
};

}

#endif
