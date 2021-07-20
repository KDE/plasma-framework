/*
    SPDX-FileCopyrightText: 2008 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "debug_p.h"
#include <QDebug>
#include <plasma/version.h>

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 85)

namespace Plasma
{
unsigned int version()
{
    return PLASMA_VERSION;
}

unsigned int versionMajor()
{
    return PLASMA_VERSION_MAJOR;
}

unsigned int versionMinor()
{
    return PLASMA_VERSION_MINOR;
}

unsigned int versionRelease()
{
    return PLASMA_VERSION_PATCH;
}

const char *versionString()
{
    return PLASMA_VERSION_STRING;
}

bool isPluginVersionCompatible(unsigned int version)
{
    if (version == quint32(-1)) {
        // unversioned, just let it through
        qCWarning(LOG_PLASMA) << "unversioned plugin detected, may result in instability";
        return true;
    }

    // we require PLASMA_VERSION_MAJOR and PLASMA_VERSION_MINOR
    const quint32 minVersion = PLASMA_MAKE_VERSION(PLASMA_VERSION_MAJOR, 0, 0);
    const quint32 maxVersion = PLASMA_MAKE_VERSION(PLASMA_VERSION_MAJOR, PLASMA_VERSION_MINOR, 60);

    if (version < minVersion || version > maxVersion) {
#ifndef NDEBUG
        // qCDebug(LOG_PLASMA) << "plugin is compiled against incompatible Plasma version  " << version
        //         << "This build is compatible with" << PLASMA_VERSION_MAJOR << ".0.0 (" << minVersion
        //         << ") to" << PLASMA_VERSION_STRING << "(" << maxVersion << ")";
#endif
        return false;
    }

    return true;
}

} // Plasma namespace
#endif
