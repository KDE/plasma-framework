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
#include <QFile>

#include <kdeclarative/kdeclarative.h>

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

    //FIXME: probably needed for QmldirFile as well.
    //at the moment a qt bug prevents intercept() working with qmldirs
    //see https://codereview.qt-project.org/#change,61208
    if (type != QQmlAbstractUrlInterceptor::QmldirFile) {

        //asked a file inside a package: let's rewrite the url!
        if (path.path().startsWith(m_package.path())) {
            //qDebug() << "Found URL in package" << path;

            //tries to isolate the relative path asked relative to the contentsPrefixPath: like ui/foo.qml
            QString relativePath;
            foreach (const QString &prefix, m_package.contentsPrefixPaths()) {
                if (path.path().startsWith(m_package.path()+prefix)) {
                    //obtain a string in the form ui/foo/bar/baz.qml
                    relativePath = path.path().mid(QString(m_package.path()+prefix).length());
                    break;
                }
            }
            //should never happen
            Q_ASSERT(!relativePath.isEmpty());

            QStringList components = relativePath.split("/");
            //a path with less than 2 items should ever happen
            Q_ASSERT(components.count() >= 2);

            components.pop_front();
            //obtain a string in the form foo/bar/baz.qml: ui/ gets discarded
            QString filename = components.join("/");

            //qDebug() << "Returning" << QUrl::fromLocalFile(m_package.filePath(prefixForType(type, filename), filename));
            return QUrl::fromLocalFile(m_package.filePath(prefixForType(type, filename), filename));

        //forbid to load random absolute paths
        } else {
            foreach (const QString &import, m_engine->importPathList()) {
                //it's from an import, good
                //TODO: implement imports whitelist?
                if (path.path().startsWith(import)) {
                    qDebug() << "Found import, access granted" << path;

                    //check if there is a platform specific file that overrides this import
                    foreach (const QString &platform, KDeclarative::runtimePlatform()) {
                        qDebug() << "Trying" << platform;

                        //search for a platformqml/ path sibling of this import path
                        QString platformPath = import+"/../platformqml/"+platform+path.path().mid(import.length());
                        QFile f(platformPath);

                        qDebug() << "Found a platform specific file:" << QUrl::fromLocalFile(platformPath)<<f.exists();
                        if (f.exists()) {
                            return QUrl::fromLocalFile(platformPath);
                        }
                    }
                    return path;
                }
            }
            qWarning() << "WARNING: Access denied for URL" << path;
        }
    }

    return path;
}



