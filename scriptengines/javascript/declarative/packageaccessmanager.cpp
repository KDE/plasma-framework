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

#include "packageaccessmanager.h"

#include <Plasma/Package>

PackageAccessManager::PackageAccessManager(const Plasma::Package *package, QObject *parent)
    : KIO::AccessManager(parent),
      m_package(package)
{
}

PackageAccessManager::~PackageAccessManager()
{
}


QNetworkReply *PackageAccessManager::createRequest(Operation op, const QNetworkRequest &req, QIODevice *outgoingData)
{
    QUrl reqUrl(req.url());

    if (reqUrl.scheme() == "plasmapackage") {
        QNetworkRequest request = req;
        reqUrl.setScheme("file");
        reqUrl.setPath(m_package->path()+"/contents/"+reqUrl.path());
        request.setUrl(reqUrl);
        return KIO::AccessManager::createRequest(op, request, outgoingData);
    } else {
        return KIO::AccessManager::createRequest(op, req, outgoingData);
    }
}



