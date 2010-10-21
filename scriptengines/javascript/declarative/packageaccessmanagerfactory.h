/*
 *   Copyright 2010 Marco Martin <notmart@gmail.com>
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

#ifndef PACKAGEACCESSMANAGERFACTORY_H
#define PACKAGEACCESSMANAGERFACTORY_H

#include <QDeclarativeNetworkAccessManagerFactory>

namespace Plasma
{
    class Package;
}

class PackageAccessManagerFactory : public QDeclarativeNetworkAccessManagerFactory
{
public:
    PackageAccessManagerFactory(const Plasma::Package *package);
    ~PackageAccessManagerFactory();
    QNetworkAccessManager *create(QObject *parent);

private:
    const Plasma::Package *m_package;
};

#endif
