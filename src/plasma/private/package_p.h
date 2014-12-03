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

    QString servicePrefix;
    KPackage::Package *internalPackage;
    Package *fallbackPackage;
    PackageStructure *structure;
};

}

#endif
