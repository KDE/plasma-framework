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

#include <QNetworkReply>

#include <Plasma/Package>

#include "plasmoid/appletauthorization.h"

class ErrorReply : public QNetworkReply
{
public:
    ErrorReply(QNetworkAccessManager::Operation op, const QNetworkRequest &req)
        : QNetworkReply()
    {
        setError(QNetworkReply::ContentOperationNotPermittedError, "The plasmoid has not been authorized to load remote content");
        setOperation(op);
        setRequest(req);
        setUrl(req.url());
    }

    qint64 readData(char *data, qint64 maxSize)
    {
        return 0;
    }

    void abort()
    {
    }
};

PackageAccessManager::PackageAccessManager(const Plasma::Package *package, AppletAuthorization *auth, QObject *parent)
    : KIO::AccessManager(parent),
      m_package(package),
      m_auth(auth)
{
}

PackageAccessManager::~PackageAccessManager()
{
}


QNetworkReply *PackageAccessManager::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &req, QIODevice *outgoingData)
{
    QUrl reqUrl(req.url());

    if (reqUrl.scheme() == "plasmapackage") {
        QNetworkRequest request = req;
        reqUrl.setScheme("file");
        reqUrl.setPath(m_package->filePath(0, reqUrl.path()));
        request.setUrl(reqUrl);
        return QNetworkAccessManager::createRequest(op, request, outgoingData);
    } else if ((reqUrl.scheme() == "http" && !m_auth->authorizeRequiredExtension("http")) ||
               ((reqUrl.scheme() == "file" || reqUrl.scheme() == "desktop") && !m_auth->authorizeRequiredExtension("localio")) ||
               (!m_auth->authorizeRequiredExtension("networkio"))) {
        return new ErrorReply(op, req);
    } else {
        return KIO::AccessManager::createRequest(op, req, outgoingData);
#ifndef PLASMA_NO_KIO
            return KIO::AccessManager::createRequest(op, req, outgoingData);
#else
            return QNetworkAccessManager::createRequest(op, req, outgoingData);
#endif
    }
}



