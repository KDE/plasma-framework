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

#include <plasma/version.h>

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
    return PLASMA_VERSION_RELEASE;
}

const char *versionString()
{
    return PLASMA_VERSION_STRING;
}

} // Plasma namespace

