/*
 *   Copyright © 2009 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
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

#include "../plasma.h"
#include "../package.h"
#include "../service.h"

#include <QString>

namespace Plasma
{

class PackagePrivate
{
public:
    PackagePrivate(const PackageStructure::Ptr st, const QString &p);
    ~PackagePrivate();

    void publish(AnnouncementMethods methods);
    void unpublish();
    bool isPublished() const;

#ifdef QCA2_FOUND
    void updateHash(const QString &basePath, const QString &subPath, const QDir &dir, QCA::Hash &hash);
#endif

    PackageStructure::Ptr structure;
    Service *service;
    bool valid;
};

}

#endif
