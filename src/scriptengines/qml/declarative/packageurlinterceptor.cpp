/*
 *   Copyright 2013 Marco Martin <notmart@gmail.com>
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

#include "packageurlinterceptor.h"

#include <QDebug>
#include <QQmlEngine>

PackageUrlInterceptor::PackageUrlInterceptor(QQmlEngine *engine, const Plasma::Package &p)
    : QQmlAbstractUrlInterceptor(),
      m_package(p),
      m_engine(engine)
{
}

PackageUrlInterceptor::~PackageUrlInterceptor()
{
}

QUrl PackageUrlInterceptor::intercept(const QUrl &path, QQmlAbstractUrlInterceptor::DataType type)
{
    //qDebug() << "Intercepted URL:" << path;

    //TODO: security: permission for remote urls
    if (!path.isLocalFile() ) {
        return path;
    }

    switch (type) {
    case QQmlAbstractUrlInterceptor::QmlFile:
        if (path.path().startsWith(m_package.path())) {
            //qDebug() << "Found URL in package" << path;
            QStringList components = path.toLocalFile().split("/");
            if (components.count() < 2) {
                return path;
            }
            QString filename = components.last();
            components.pop_back();
            QString type = components.last();
            if (type == "ui" || type == "config") {
                //qDebug() << "Returning" << QUrl::fromLocalFile(m_package.filePath(type.toLatin1(), filename));
                return QUrl::fromLocalFile(m_package.filePath(type.toLatin1(), filename));
            }
        //forbid to load random absolute paths
        } else {
            foreach (const QString &import, m_engine->importPathList()) {
                //it's from an import, good
                //TODO: implement imports whitelist?
                if (path.path().startsWith(import)) {
                   // qDebug() << "Found import, access granted";
                    return path;
                }
            }
            qWarning() << "Access denied for url" << path;
        }
        break;

    default:
        break;
    }

    return path;
}



