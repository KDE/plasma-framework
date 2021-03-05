/*
    SPDX-FileCopyrightText: 2011 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef APPBACKGROUND_PROVIDER_H
#define APPBACKGROUND_PROVIDER_H

#include <QQuickImageProvider>

namespace Plasma
{
class Theme;
}

// TODO: should be possible to remove without significant breakages as is just an image provider
/**
 * image provider for textures used for applications
 * @deprecated don't use it
 */
class AppBackgroundProvider : public QQuickImageProvider
{
public:
    AppBackgroundProvider();
    ~AppBackgroundProvider() override;
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    Plasma::Theme *m_theme;
};

#endif
