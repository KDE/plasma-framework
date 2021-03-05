/*
    SPDX-FileCopyrightText: 2011 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "appbackgroundprovider_p.h"

#include <QLatin1String>
#include <QPixmap>
#include <QSize>
#include <QStandardPaths>

#include <Plasma/Theme>

AppBackgroundProvider::AppBackgroundProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
    m_theme = new Plasma::Theme();
}

AppBackgroundProvider::~AppBackgroundProvider()
{
    m_theme->deleteLater();
}

QImage AppBackgroundProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(size)
    Q_UNUSED(requestedSize)
    return QImage(m_theme->backgroundPath(id % QStringLiteral(".png")));
}
