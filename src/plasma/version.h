/*
    SPDX-FileCopyrightText: 2008 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMAVERSION_H
#define PLASMAVERSION_H

/** @file plasma/version.h <Plasma/Version> */

#include <plasma/plasma_export.h>

#if PLASMA_ENABLE_DEPRECATED_SINCE(5, 85)

#include <plasma_version.h>

#define PLASMA_MAKE_VERSION(a, b, c) (((a) << 16) | ((b) << 8) | (c))

/**
 * Compile-time macro for checking the plasma version. Not useful for
 * detecting the version of libplasma at runtime.
 * @deprecated Since 5.85, use plasma_version.h header instead
 */
#define PLASMA_IS_VERSION(a, b, c) (PLASMA_VERSION >= PLASMA_MAKE_VERSION(a, b, c))

/**
 * Namespace for everything in libplasma
 */
namespace Plasma
{
/**
 * The runtime version of libplasma
 * @deprecated Since 5.85, use plasma_version.h header instead
 */
PLASMA_DEPRECATED_VERSION(5, 85, "use plasma_version.h header instead")
PLASMA_EXPORT unsigned int version();

/**
 * The runtime major version of libplasma
 * @deprecated Since 5.85, use plasma_version.h header instead
 */
PLASMA_DEPRECATED_VERSION(5, 85, "use plasma_version.h header instead")
PLASMA_EXPORT unsigned int versionMajor();

/**
 * The runtime major version of libplasma
 * @deprecated Since 5.85, use plasma_version.h header instead
 */
PLASMA_DEPRECATED_VERSION(5, 85, "use plasma_version.h header instead")
PLASMA_EXPORT unsigned int versionMinor();

/**
 * The runtime major version of libplasma
 * @deprecated Since 5.85, use plasma_version.h header instead
 */
PLASMA_DEPRECATED_VERSION(5, 85, "use plasma_version.h header instead")
PLASMA_EXPORT unsigned int versionRelease();

/**
 * The runtime version string of libplasma
 * @deprecated Since 5.85, use plasma_version.h header instead
 */
PLASMA_DEPRECATED_VERSION(5, 85, "use plasma_version.h header instead")
PLASMA_EXPORT const char *versionString();

/**
 * Verifies that a plugin is compatible with plasma
 * @deprecated Since 5.85, method is obsolete with the deprecation of KPluginLoader::pluginVersion.
 * Consider using a versioned namespace or KPluginMetaData::version instead.
 */
PLASMA_DEPRECATED_VERSION(5, 85, "See API docs")
PLASMA_EXPORT bool isPluginVersionCompatible(unsigned int version);

} // Plasma namespace

#endif
#endif // multiple inclusion guard
