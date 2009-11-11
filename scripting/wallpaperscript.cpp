/*
 *   Copyright 2009 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2009 by Petri Damsten <damu@iki.fi>
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

#include "wallpaperscript.h"
#include "private/wallpaper_p.h"
#include "package.h"

namespace Plasma
{

class WallpaperScriptPrivate
{
public:
    Wallpaper *wallpaper;
};

WallpaperScript::WallpaperScript(QObject *parent)
    : ScriptEngine(parent),
      d(new WallpaperScriptPrivate)
{
}

WallpaperScript::~WallpaperScript()
{
    delete d;
}

void WallpaperScript::setWallpaper(Wallpaper *wallpaper)
{
    d->wallpaper = wallpaper;
    connect(wallpaper, SIGNAL(renderCompleted(const QImage&)),
            this, SLOT(renderCompleted(const QImage&)));
    connect(wallpaper, SIGNAL(urlDropped(const KUrl&)),
            this, SLOT(urlDropped(const KUrl&)));
}

Wallpaper *WallpaperScript::wallpaper() const
{
    return d->wallpaper;
}

QString WallpaperScript::mainScript() const
{
    Q_ASSERT(d->wallpaper);
    return d->wallpaper->package()->filePath("mainscript");
}

const Package *WallpaperScript::package() const
{
    Q_ASSERT(d->wallpaper);
    return d->wallpaper->package();
}

KPluginInfo WallpaperScript::description() const
{
    Q_ASSERT(d->wallpaper);
    return d->wallpaper->d->wallpaperDescription;
}

void WallpaperScript::initWallpaper(const KConfigGroup &config)
{
    Q_UNUSED(config)
}

void WallpaperScript::paint(QPainter *painter, const QRectF &exposedRect)
{
    Q_UNUSED(painter)
    Q_UNUSED(exposedRect)
}

void WallpaperScript::save(KConfigGroup &config)
{
    Q_UNUSED(config)
}

QWidget *WallpaperScript::createConfigurationInterface(QWidget *parent)
{
    Q_UNUSED(parent)
    return 0;
}

void WallpaperScript::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
}

void WallpaperScript::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
}

void WallpaperScript::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
}

void WallpaperScript::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    Q_UNUSED(event)
}

bool WallpaperScript::isInitialized() const
{
    if (d->wallpaper) {
        return d->wallpaper->isInitialized();
    }
    return false;
}

QRectF WallpaperScript::boundingRect() const
{
    if (d->wallpaper) {
        return d->wallpaper->boundingRect();
    }
    return QRectF();
}

DataEngine *WallpaperScript::dataEngine(const QString &name) const
{
    Q_ASSERT(d->wallpaper);
    return d->wallpaper->dataEngine(name);
}

void WallpaperScript::setResizeMethodHint(Wallpaper::ResizeMethod resizeMethod)
{
    if (d->wallpaper) {
        d->wallpaper->setResizeMethodHint(resizeMethod);
    }
}

void WallpaperScript::setTargetSizeHint(const QSizeF &targetSize)
{
    if (d->wallpaper) {
        d->wallpaper->setTargetSizeHint(targetSize);
    }
}

void WallpaperScript::setConfigurationRequired(bool needsConfiguring, const QString &reason)
{
    if (d->wallpaper) {
        d->wallpaper->setConfigurationRequired(needsConfiguring, reason);
    }
}

void WallpaperScript::render(const QString &sourceImagePath, const QSize &size,
        Wallpaper::ResizeMethod resizeMethod, const QColor &color)
{
    if (d->wallpaper) {
        d->wallpaper->render(sourceImagePath, size, resizeMethod, color);
    }
}

void WallpaperScript::setUsingRenderingCache(bool useCache)
{
    if (d->wallpaper) {
        d->wallpaper->setUsingRenderingCache(useCache);
    }
}

bool WallpaperScript::findInCache(const QString &key, QImage &image, unsigned int lastModified)
{
    if (d->wallpaper) {
        return d->wallpaper->findInCache(key, image, lastModified);
    }
    return false;
}

void WallpaperScript::insertIntoCache(const QString& key, const QImage &image)
{
    if (d->wallpaper) {
        d->wallpaper->insertIntoCache(key, image);
    }
}

void WallpaperScript::setContextualActions(const QList<QAction*> &actions)
{
    if (d->wallpaper) {
        d->wallpaper->setContextualActions(actions);
    }
}

void WallpaperScript::update(const QRectF &exposedArea)
{
    if (d->wallpaper) {
        d->wallpaper->update(exposedArea);
    }
}

void WallpaperScript::configNeedsSaving()
{
    if (d->wallpaper) {
        d->wallpaper->configNeedsSaving();
    }
}

void WallpaperScript::renderCompleted(const QImage &image)
{
    Q_UNUSED(image)
}

void WallpaperScript::urlDropped(const KUrl &url)
{
    Q_UNUSED(url)
}

} // Plasma namespace

#include "wallpaperscript.moc"
