/*
    SPDX-FileCopyrightText: 2007-2009 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plasma.h"

#include <KLocalizedString>
#include <kpackage/package.h>
#include <kpackage/packagestructure.h>
#include "packages_p.h"

K_EXPORT_KPACKAGE_PACKAGE_WITH_JSON(Plasma::GenericPackage, "plasmageneric-packagestructure.json")

#include "plasmagenericpackage.moc"
