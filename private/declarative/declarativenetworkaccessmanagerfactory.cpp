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

#include "declarativenetworkaccessmanagerfactory_p.h"

#include "config-plasma.h"

#ifndef PLASMA_NO_KIO
#include <kio/accessmanager.h>
#else
#include <QtNetwork/QNetworkAccessManager>
#endif

QNetworkAccessManager *DeclarativeNetworkAccessManagerFactory::create(QObject *parent)
{
#ifndef PLASMA_NO_KIO
    return new KIO::AccessManager(parent);
#else
    return new QNetworkAccessManager(parent);
#endif
}


