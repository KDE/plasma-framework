/*
    SPDX-FileCopyrightText: 2007-2009 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plasma.h"

#include "packages_p.h"
#include <KLocalizedString>
#include <kpackage/package.h>
#include <kpackage/packagestructure.h>

using namespace Plasma;
K_PLUGIN_CLASS_WITH_JSON(GenericPackage, "plasma_generic_packagestructure.json")

#include "plasma_generic_packagestructure.moc"
