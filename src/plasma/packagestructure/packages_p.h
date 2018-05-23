/******************************************************************************
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>                        *
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
    ChangeableMainScriptPackage(QObject *parent = nullptr, const QVariantList &args = QVariantList()) : KPackage::PackageStructure(parent, args) {}

    void initPackage(KPackage::Package *package) override;
    void pathChanged(KPackage::Package *package) override;

protected:
    virtual QString mainScriptConfigKey() const;
};

class GenericPackage : public ChangeableMainScriptPackage
{
    Q_OBJECT
public:
    GenericPackage(QObject *parent = nullptr, const QVariantList &args = QVariantList()) : ChangeableMainScriptPackage(parent, args) {}
    void initPackage(KPackage::Package *package) override;
};

} // namespace Plasma

#endif // LIBS_PLASMA_PACKAGES_P_H
