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

namespace PlasmaQuick
{

PackageUrlInterceptor::PackageUrlInterceptor(QQmlEngine *engine, const Plasma::Package &p)
    : QQmlAbstractUrlInterceptor(),
      m_package(p),
      m_engine(engine)
{
    m_allowedPaths << m_engine->importPathList();
}

PackageUrlInterceptor::~PackageUrlInterceptor()
{
}

void PackageUrlInterceptor::addAllowedPath(const QString &path)
{
    m_allowedPaths << path;
}

void PackageUrlInterceptor::removeAllowedPath(const QString &path)
{
    m_allowedPaths.removeAll(path);
}

QStringList PackageUrlInterceptor::allowedPaths() const
{
    return m_allowedPaths;
}

QUrl PackageUrlInterceptor::intercept(const QUrl &path, QQmlAbstractUrlInterceptor::DataType type)
{
    //qDebug() << "Intercepted URL:" << path << type;

    if (path.scheme() == QStringLiteral("plasmapackage")) {
        //FIXME: this is incorrect but works around a bug in qml in resolution of urls of qmldir files
        if (type == QQmlAbstractUrlInterceptor::QmldirFile) {
            return QUrl(m_package.filePath(0, path.path()));
        } else {
            return QUrl::fromLocalFile(m_package.filePath(0, path.path()));
        }
    }

    //TODO: security: permission for remote urls
    if (!path.isLocalFile()) {
        return path;
    }

    //if is just a normal string, no qml file was asked, allow it
    if (type == QQmlAbstractUrlInterceptor::UrlString) {
        return path;
    }

    //asked a file inside a package: let's rewrite the url!
    if (path.path().startsWith(m_package.path())) {
        //qDebug() << "Found URL in package" << path;

        //tries to isolate the relative path asked relative to the contentsPrefixPath: like ui/foo.qml
        QString relativePath;
        foreach (const QString &prefix, m_package.contentsPrefixPaths()) {
            if (path.path().startsWith(m_package.path() + prefix)) {
                //obtain a string in the form ui/foo/bar/baz.qml
                relativePath = path.path().mid(QString(m_package.path() + prefix).length());
                break;
            }
        }
        //should never happen
        Q_ASSERT(!relativePath.isEmpty());

        QStringList components = relativePath.split(QLatin1Char('/'));
        //a path with less than 2 items should ever happen
        Q_ASSERT(components.count() >= 2);

        components.pop_front();
        //obtain a string in the form foo/bar/baz.qml: ui/ gets discarded
        const QString &filename = components.join("/");

        //qDebug() << "Returning" << QUrl::fromLocalFile(m_package.filePath(prefixForType(type, filename), filename));
        return QUrl::fromLocalFile(m_package.filePath(prefixForType(type, filename), filename));

        //forbid to load random absolute paths
    } else {
        foreach (const QString &allowed, m_allowedPaths) {
            //It's a private import
            if (path.path().contains("org/kde/plasma/private")) {
                QString pathCheck(path.path());
                pathCheck = pathCheck.replace(QRegExp(".*org/kde/plasma/private/(.*)/.*"), "org.kde.plasma.\\1");

                if (pathCheck == m_package.metadata().pluginName()) {
                    return path;
                } else {
                    return QUrl("file://" + allowed + "/org/kde/plasma/accessdenied/qmldir");
                }
            }
            //it's from an allowed, good
            if (path.path().startsWith(allowed)) {
                //qDebug() << "Found allowed, access granted" << path;

                //check if there is a platform specific file that overrides this allowed
                foreach (const QString &platform, KDeclarative::KDeclarative::runtimePlatform()) {
                    //qDebug() << "Trying" << platform;

                    //search for a platformqml/ path sibling of this allowed path
                    const QString &platformPath = allowed + QStringLiteral("/../platformqml/") + platform + path.path().mid(allowed.length());
                    const QFile f(platformPath);

                    //qDebug() << "Found a platform specific file:" << QUrl::fromLocalFile(platformPath)<<f.exists();
                    if (f.exists()) {
                        return QUrl::fromLocalFile(platformPath);
                    }
                }
                return path;
            }
        }
    }

    qWarning() << "WARNING: Access denied for URL" << path << m_package.path();
    return QUrl();
}

}

