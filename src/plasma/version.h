/*
 *   Copyright 2008 by Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#ifndef PLASMAVERSION_H
#define PLASMAVERSION_H

/** @file plasma/version.h <Plasma/Version> */

#include "plasma_export.h"
#include <plasma_version.h>

#define PLASMA_MAKE_VERSION(a,b,c) (((a) << 16) | ((b) << 8) | (c))

/**
 * Compile-time macro for checking the plasma version. Not useful for
 * detecting the version of libplasma at runtime.
 */
#define PLASMA_IS_VERSION(a,b,c) (PLASMA_VERSION >= PLASMA_MAKE_VERSION(a,b,c))

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
