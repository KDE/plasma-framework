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
#include <QFileInfo>
#include <QStandardPaths>

#include <Plasma/PluginLoader>
#include <Plasma/Package>
#include <KPackage/Package>

#include <kdeclarative/kdeclarative.h>

namespace PlasmaQuick
{

class PackageUrlInterceptorPrivate {
public:
    PackageUrlInterceptorPrivate(QQmlEngine *engine, const KPackage::Package &p)
        : package(p),
          engine(engine)
    {
    }

    KPackage::Package package;
    QStringList allowedPaths;
    QQmlEngine *engine;
    bool forcePlasmaStyle = false;

    //FIXME: those are going to be stuffed here and stay..
    // they should probably be removed when the last applet of that type is removed
    static QHash<QString, KPackage::Package> s_packages;
};

QHash<QString, KPackage::Package> PackageUrlInterceptorPrivate::s_packages = QHash<QString, KPackage::Package>();


PackageUrlInterceptor::PackageUrlInterceptor(QQmlEngine *engine, const KPackage::Package &p)
    : QQmlAbstractUrlInterceptor(),
      d(new PackageUrlInterceptorPrivate(engine, p))
{
    //d->allowedPaths << d->engine->importPathList();
}

PackageUrlInterceptor::~PackageUrlInterceptor()
{
    delete d;
}

void PackageUrlInterceptor::addAllowedPath(const QString &path)
{
    d->allowedPaths << path;
}

void PackageUrlInterceptor::removeAllowedPath(const QString &path)
{
    d->allowedPaths.removeAll(path);
}

QStringList PackageUrlInterceptor::allowedPaths() const
{
    return d->allowedPaths;
}

bool PackageUrlInterceptor::forcePlasmaStyle() const
{
    return d->forcePlasmaStyle;
}

void PackageUrlInterceptor::setForcePlasmaStyle(bool force)
{
    d->forcePlasmaStyle = force;
}

QUrl PackageUrlInterceptor::intercept(const QUrl &path, QQmlAbstractUrlInterceptor::DataType type)
{
    //qDebug() << "Intercepted URL:" << path << type;

    if (d->forcePlasmaStyle && path.path().contains(QLatin1String("Controls.2/org.kde.desktop/"))) {
        return QUrl::fromLocalFile(path.path().replace(QLatin1String("Controls.2/org.kde.desktop/"), QLatin1String("Controls.2/Plasma/")));
    }
    QString pkgRoot;
    KPackage::Package package;
    if (d->package.isValid()) {
        package = d->package;
    } else {
        foreach (const QString &base, QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation)) {
            pkgRoot = QFileInfo(base + QStringLiteral("/plasma/plasmoids/")).canonicalFilePath();
            if (!pkgRoot.isEmpty() && path.path().startsWith(pkgRoot)) {
                const QString pkgName = path.path().midRef(pkgRoot.length() + 1).split(QLatin1Char('/')).first().toString();
#warning FIX double look-up
                if (PackageUrlInterceptorPrivate::s_packages.contains(pkgName)) {
                    package = PackageUrlInterceptorPrivate::s_packages.value(pkgName);
                } else {
                    package = Plasma::PluginLoader::self()->loadPackage(QStringLiteral("Plasma/Applet")).kPackage();
                    package.setPath(pkgName);
                    PackageUrlInterceptorPrivate::s_packages[pkgName] = package;
                }
                break;
            }
        }
    }
    if (!package.isValid()) {
        return path;
    }

    if (d->package.isValid() && path.scheme() == QStringLiteral("plasmapackage")) {
        //FIXME: this is incorrect but works around a bug in qml in resolution of urls of qmldir files
        if (type == QQmlAbstractUrlInterceptor::QmldirFile) {
            return QUrl(d->package.filePath(0, path.path()));
        } else {
            return QUrl::fromLocalFile(d->package.filePath(0, path.path()));
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
    if (path.path().startsWith(package.path())) {
        //qDebug() << "Found URL in package" << path;

        //tries to isolate the relative path asked relative to the contentsPrefixPath: like ui/foo.qml
        QString relativePath;
        foreach (const QString &prefix, package.contentsPrefixPaths()) {
            QString root = package.path() + prefix;
            if (path.path().startsWith(root)) {
                //obtain a string in the form ui/foo/bar/baz.qml
                relativePath = path.path().mid(root.length());
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
        const QString &filename = components.join(QStringLiteral("/"));

        QUrl ret = QUrl::fromLocalFile(package.filePath(prefixForType(type, filename), filename));

        //qDebug() << "Returning" << ret;

        if (ret.path().isEmpty()) {
            return path;
        }
        return ret;

        //forbid to load random absolute paths
    }

    return path;
}

}

