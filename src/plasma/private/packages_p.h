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

#include "packagestructure.h"
#include "plasma.h"

namespace Plasma
{

class ChangeableMainScriptPackage : public PackageStructure
{
public:
    void initPackage(Package *package);

protected:
    virtual QString mainScriptConfigKey() const;
    void pathChanged(Package *package);
};

class GenericPackage : public ChangeableMainScriptPackage
{
public:
    void initPackage(Package *package);
};

class PlasmoidPackage : public GenericPackage
{
public:
    void initPackage(Package *package);
};

class DataEnginePackage : public ChangeableMainScriptPackage
{
public:
    void initPackage(Package *package);
};

class ThemePackage : public PackageStructure
{
public:
    void initPackage(Package *package);
};

class ContainmentActionsPackage : public ChangeableMainScriptPackage
{
public:
    void initPackage(Package *package);
};

class ShellPackage: public Plasma::PackageStructure
{
public:
    void initPackage(Plasma::Package *package);
};

} // namespace Plasma

#endif // LIBS_PLASMA_PACKAGES_P_H
