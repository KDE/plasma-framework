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

#ifndef PACKAGEACCESSMANAGER_H
#define PACKAGEACCESSMANAGER_H


#ifndef PLASMA_NO_KIO
#include <kio/accessmanager.h>
#else
#include <QtNetwork/QNetworkAccessManager>
#endif

namespace Plasma
{
    class Package;
}

class AppletAuthorization;

class PackageAccessManager :
#ifdef PLASMA_NO_KIO
public QNetworkAccessManager
#else
public KIO::AccessManager
#endif
{
public:
    PackageAccessManager(const Plasma::Package *package, AppletAuthorization *auth, QObject *parent = 0);
    ~PackageAccessManager();

protected:
    QNetworkReply *createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &req, QIODevice *outgoingData = 0);

private:
    const Plasma::Package *m_package;
    AppletAuthorization *m_auth;
};

#endif
