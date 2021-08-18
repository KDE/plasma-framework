/*
    SPDX-FileCopyrightText: 2013 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "packageurlinterceptor.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QFileSelector>
#include <QQmlEngine>
#include <QStandardPaths>

#include <Plasma/Package>
#include <Plasma/PluginLoader>

#include <kdeclarative/kdeclarative.h>

namespace PlasmaQuick
{
class PackageUrlInterceptorPrivate
{
public:
    PackageUrlInterceptorPrivate(QQmlEngine *engine, PackageUrlInterceptor *interceptor, const KPackage::Package &p)
        : q(interceptor)
        , package(p)
        , engine(engine)
    {
        selector = new QFileSelector;
    }

    ~PackageUrlInterceptorPrivate()
    {
        engine->setUrlInterceptor(nullptr);
        delete selector;
    }

    PackageUrlInterceptor *q;
    KPackage::Package package;
    QStringList allowedPaths;
    QQmlEngine *engine;
    QFileSelector *selector;
    bool forcePlasmaStyle = false;
};

PackageUrlInterceptor::PackageUrlInterceptor(QQmlEngine *engine, const KPackage::Package &p)
    : QQmlAbstractUrlInterceptor()
    , d(new PackageUrlInterceptorPrivate(engine, this, p))
{
    // d->allowedPaths << d->engine->importPathList();
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
    // qDebug() << "Intercepted URL:" << path << type;

    const QString urlPath = path.path();
    // Don't intercept qmldir files, to prevent double interception
    if (urlPath.endsWith(QLatin1String("qmldir"))) {
        return path;
    }
    // We assume we never rewritten qml/qmldir files
    if (urlPath.endsWith(QLatin1String("qml")) || urlPath.endsWith(QLatin1String("/inline"))) {
        return d->selector->select(path);
    }
    const QString prefix = QString::fromUtf8(prefixForType(type, urlPath));
    // TODO KF6: Kill this hack
    const QLatin1String marker("/ui/");
    QString plainPath = path.toString();
    const int index = plainPath.indexOf(marker);
    if (index != -1) {
        QStringView strView(plainPath);

        plainPath = strView.left(index) + QLatin1Char('/') + prefix + QLatin1Char('/') + strView.mid(index + marker.size());

        const QUrl url = QUrl(plainPath);
        const QString newPath = url.path();
        // search it in a resource or as a file on disk
        if (!(plainPath.contains(QLatin1String("qrc")) && QFile::exists(QLatin1Char(':') + newPath)) //
            && !QFile::exists(newPath)) {
            return d->selector->select(path);
        }
        qWarning() << "Warning: all files used by qml by the plasmoid should be in ui/. The file in the path" << plainPath << "was expected at" << path;
        // This deprecated code path doesn't support selectors
        return url;
    }
    return d->selector->select(path);
}

}
