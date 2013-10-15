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

#ifndef PACKAGEURLINTERCEPTOR_H
#define PACKAGEURLINTERCEPTOR_H

#include <QQmlAbstractUrlInterceptor>
#include <QQmlNetworkAccessManagerFactory>

#include <plasmaquick/plasmaquick_export.h>

#include <Plasma/Package>

class QQmlEngine;

//depends from https://codereview.qt-project.org/#change,65626
class PLASMAQUICK_EXPORT PackageUrlInterceptor: public QQmlAbstractUrlInterceptor
{
public:
    PackageUrlInterceptor(QQmlEngine *engine, const Plasma::Package &p);
    virtual ~PackageUrlInterceptor();

    virtual QUrl intercept(const QUrl &path, QQmlAbstractUrlInterceptor::DataType type);

    static QQmlNetworkAccessManagerFactory* createPackageAccessManagerFactory(const Plasma::Package &package);

    static inline QByteArray prefixForType(QQmlAbstractUrlInterceptor::DataType type, const QString &fileName)
    {
        switch (type) {
        case QQmlAbstractUrlInterceptor::QmlFile:
            return QByteArray("ui");
        case QQmlAbstractUrlInterceptor::JavaScriptFile:
            return QByteArray("scripts");
        default:
            break;
        }

        //failed by type, let's try by extension
        const QString &extension = fileName.mid(fileName.lastIndexOf(QLatin1Char('.')) + 1).toLower();

        if (extension == QStringLiteral("svg") || extension == QStringLiteral("svgz") ||
            extension == QStringLiteral("png") || extension == QStringLiteral("gif") ||
            extension == QStringLiteral("jpg") || extension == QStringLiteral("jpeg")) {
            return QByteArray("images");
        //FIXME: are those necessary? are they *always* catched by type?
        } else if (extension == QStringLiteral("js")) {
            return QByteArray("scripts");
        } else if (extension == QStringLiteral("qml")) {
            return QByteArray("ui");
        //everything else, throw it in "data"
        } else {
            return QByteArray("data");
        }
    }

private:
    Plasma::Package m_package;
    QQmlEngine *m_engine;
};


#endif
