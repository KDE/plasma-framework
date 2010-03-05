/*
 *   Copyright 2008 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Petri Damsten <damu@iki.fi>
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

#ifndef PLASMA_WALLPAPERPRIVATE_H
#define PLASMA_WALLPAPERPRIVATE_H

#include <QtCore/QQueue>
#include <QtCore/QRunnable>
#include <QtCore/QWeakPointer>

#include "plasma/scripting/wallpaperscript.h"
#include "plasma/private/dataengineconsumer_p.h"
#include "plasma/private/wallpaperrenderthread_p.h"

namespace Plasma
{

class WallpaperPrivate : public DataEngineConsumer
{
public:
    class RenderRequest
    {
        public:
            QWeakPointer<Wallpaper> parent;
            QString file;
            QSize size;
            Wallpaper::ResizeMethod resizeMethod;
            QColor color;
    };

    WallpaperPrivate(KService::Ptr service, Wallpaper *wallpaper);

    QString cachePath(const QString &key) const;
    QString cacheKey(const QString &sourceImagePath, const QSize &size,
                     int resizeMethod, const QColor &color) const;
    void initScript();

    bool findInCache(const QString &key, unsigned int lastModified = 0);

    void renderCompleted(WallpaperRenderThread *renderer,int token, const QImage &image,
                         const QString &sourceImagePath, const QSize &size,
                         int resizeMethod, const QColor &color);
    void setupScriptSupport();

    static QList<WallpaperRenderThread *> s_renderers;
    static QQueue<RenderRequest> s_renderQueue;
    static PackageStructure::Ptr s_packageStructure;

    Wallpaper *q;
    KPluginInfo wallpaperDescription;
    Package *package;
    QRectF boundingRect;
    KServiceAction mode;
    int renderToken;
    Wallpaper::ResizeMethod lastResizeMethod;
    QSizeF targetSize;
    WallpaperScript *script;
    bool cacheRendering : 1;
    bool initialized : 1;
    bool needsConfig : 1;
    bool scriptInitialized : 1;
    bool previewing : 1;
};

class LoadImageThread : public QObject, public QRunnable
{
    Q_OBJECT
    
    private:
        QString m_filePath;

    Q_SIGNALS:
        void done(const QImage &pixmap);

    public:        
        LoadImageThread(const QString &filePath);
        void run();
};

} // namespace Plasma
#endif

