/*
 *   Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>
 *   Copyright (c) 2009 Aaron Seigo <aseigo@kde.org>
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

#ifndef PLASMA_WALLPAPERRENDERTHREAD_P_H
#define PLASMA_WALLPAPERRENDERTHREAD_P_H

#include <QColor>
#include <QImage>
#include <QMutex>
#include <QQueue>
#include <QThread>
#include <QWaitCondition>

#include "plasma/wallpaper.h"

namespace Plasma
{

class WallpaperRenderRequest
{
public:
    WallpaperRenderRequest()
        : token(++s_token)
    {

    }
    QWeakPointer<QObject> requester;
    QString file;
    QSize size;
    Wallpaper::ResizeMethod resizeMethod;
    QColor color;
    int token;

    static int s_token;
};

class WallpaperRenderThread : public QThread
{
    Q_OBJECT

public:
    WallpaperRenderThread(const WallpaperRenderRequest &request, QObject *parent = 0);
    virtual ~WallpaperRenderThread();

    static void render(const WallpaperRenderRequest &request);

Q_SIGNALS:
    void done(const WallpaperRenderRequest &request, const QImage &image);

protected:
    virtual void run();

private:
    static void checkQueue();
    WallpaperRenderRequest m_request;

    bool m_abort;
    static int s_rendererCount;
    static QQueue<WallpaperRenderRequest> s_renderQueue;
};

} // namespace Plasma

Q_DECLARE_METATYPE(Plasma::WallpaperRenderRequest);
#endif // PLASMA_WALLPAPERRENDERTHREAD_P_H
