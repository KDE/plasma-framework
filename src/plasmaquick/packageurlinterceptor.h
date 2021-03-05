/*
    SPDX-FileCopyrightText: 2013 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PACKAGEURLINTERCEPTOR_H
#define PACKAGEURLINTERCEPTOR_H

#include <QQmlAbstractUrlInterceptor>

#include <plasmaquick/plasmaquick_export.h>

#include <KPackage/Package>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the public Plasma API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

class QQmlEngine;

namespace PlasmaQuick
{
class PackageUrlInterceptorPrivate;

// depends from https://codereview.qt-project.org/#change,65626
class PLASMAQUICK_EXPORT PackageUrlInterceptor : public QQmlAbstractUrlInterceptor
{
public:
    PackageUrlInterceptor(QQmlEngine *engine, const KPackage::Package &p);
    ~PackageUrlInterceptor() override;

    void addAllowedPath(const QString &path);
    void removeAllowedPath(const QString &path);
    QStringList allowedPaths() const;

    bool forcePlasmaStyle() const;
    void setForcePlasmaStyle(bool force);

    QUrl intercept(const QUrl &path, QQmlAbstractUrlInterceptor::DataType type) override;

    static inline QByteArray prefixForType(QQmlAbstractUrlInterceptor::DataType type, const QString &fileName)
    {
        switch (type) {
        case QQmlAbstractUrlInterceptor::QmlFile:
            return QByteArray("ui");
        case QQmlAbstractUrlInterceptor::JavaScriptFile:
            return QByteArray("code");
        default:
            break;
        }

        // failed by type, let's try by extension
        const QString &extension = fileName.mid(fileName.lastIndexOf(QLatin1Char('.')) + 1).toLower();

        /* clang-format off */
        if (extension == QLatin1String("svg")
            || extension == QStringLiteral("svgz")
            || extension == QLatin1String("png")
            || extension == QStringLiteral("gif")
            || extension == QLatin1String("jpg")
            || extension == QStringLiteral("jpeg")) { /* clang-format on */
            return QByteArray("images");
            // FIXME: are those necessary? are they *always* caught by type?
        } else if (extension == QLatin1String("js")) {
            return QByteArray("code");
        } else if (extension == QLatin1String("qml") || extension == QStringLiteral("qmldir")) {
            return QByteArray("ui");
            // everything else, throw it in "data"
        } else {
            return QByteArray("data");
        }
    }

private:
    PackageUrlInterceptorPrivate *const d;
};

}

#endif
