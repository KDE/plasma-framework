/*
    SPDX-FileCopyrightText: 2008 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMAVERSION_H
#define PLASMAVERSION_H

/** @file plasma/version.h <Plasma/Version> */

#include <plasma/plasma_export.h>
#include <plasma_version.h>

#define PLASMA_MAKE_VERSION(a, b, c) (((a) << 16) | ((b) << 8) | (c))

/**
 * Compile-time macro for checking the plasma version. Not useful for
 * detecting the version of libplasma at runtime.
 */
#define PLASMA_IS_VERSION(a, b, c) (PLASMA_VERSION >= PLASMA_MAKE_VERSION(a, b, c))

/**
 * Namespace for everything in libplasma
 */
namespace Plasma
{
/**
 * The runtime version of libplasma
 */
PLASMA_EXPORT unsigned int version();

/**
 * The runtime major version of libplasma
 */
PLASMA_EXPORT unsigned int versionMajor();

/**
 * The runtime major version of libplasma
 */
PLASMA_EXPORT unsigned int versionMinor();

/**
 * The runtime major version of libplasma
 */
PLASMA_EXPORT unsigned int versionRelease();

/**
 * The runtime version string of libplasma
 */
PLASMA_EXPORT const char *versionString();

/**
 * Verifies that a plugin is compatible with plasma
 */
PLASMA_EXPORT bool isPluginVersionCompatible(unsigned int version);

} // Plasma namespace

#endif // multiple inclusion guard
