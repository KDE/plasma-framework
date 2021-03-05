/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LIBS_PLASMA_PACKAGES_P_H
#define LIBS_PLASMA_PACKAGES_P_H

#include "plasma.h"

#include <kpackage/package.h>
#include <kpackage/packagestructure.h>

namespace Plasma
{
class ChangeableMainScriptPackage : public KPackage::PackageStructure
{
    Q_OBJECT
public:
    ChangeableMainScriptPackage(QObject *parent = nullptr, const QVariantList &args = QVariantList())
        : KPackage::PackageStructure(parent, args)
    {
    }

    void initPackage(KPackage::Package *package) override;
    void pathChanged(KPackage::Package *package) override;

protected:
    virtual QString mainScriptConfigKey() const;
};

class GenericPackage : public ChangeableMainScriptPackage
{
    Q_OBJECT
public:
    GenericPackage(QObject *parent = nullptr, const QVariantList &args = QVariantList())
        : ChangeableMainScriptPackage(parent, args)
    {
    }
    void initPackage(KPackage::Package *package) override;
};

} // namespace Plasma

#endif // LIBS_PLASMA_PACKAGES_P_H
