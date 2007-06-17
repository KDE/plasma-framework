/******************************************************************************
*   Copyright (C) 2007 by Riccardo Iaconelli  <ruphy@fsfe.org>                *
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

#ifndef PACKAGER_H
#define PACKAGER_H

#include <plasma_export.h>

class KTemporaryFile;

namespace Plasma
{
class PackageMetadata;
class PackagerPrivate;

class PLASMA_EXPORT Packager
{
public:
    Packager(PackageMetadata *metadata);
    ~Packager();

    void setMetadata(PackageMetadata *metadata);
    const PackageMetadata* metadata();

    // If Metadata::isComplete() returns false, the packaging won't be done
    // returns: true if successful, false otherwise
    /**
     * Creates a package of the contents
     *
     * @arg destination the path of the file to create the package as
     * @arg source the directory containing the contents to package up
     **/
    bool createPackage(const QString& destination, const QString& source);

private:
    KTemporaryFile* generateMetadata();
    class Private;
    Private * const d;
};

}
#endif
