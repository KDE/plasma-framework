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

#include "wallpaper.h"

#include <QColor>
#include <QFile>
#include <QImage>

#include <kdebug.h>
#include <kglobal.h>
#include <kservicetypetrader.h>
#include <kstandarddirs.h>

#include <version.h>

#include "plasma/private/dataengineconsumer_p.h"
#include "plasma/private/packages_p.h"
#include "plasma/private/wallpaperrenderthread_p.h"

namespace Plasma
{

class WallpaperPrivate : public DataEngineConsumer
{
public:
    WallpaperPrivate(KService::Ptr service, Wallpaper *wallpaper) :
        q(wallpaper),
        wallpaperDescription(service),
        renderToken(-1),
        cacheRendering(false),
        initialized(false),
        needsConfig(false)
    {
    };

    QString cachePath(const QString &sourceImagePath, const QSize &size,
                      Wallpaper::ResizeMethod resizeMethod, const QColor &color) const;


    void renderCompleted(int token, const QImage &image,
                         const QString &sourceImagePath, const QSize &size,
                         Wallpaper::ResizeMethod resizeMethod, const QColor &color);

    static WallpaperRenderThread s_renderer;
    static PackageStructure::Ptr packageStructure;

    Wallpaper *q;
    KPluginInfo wallpaperDescription;
    QRectF boundingRect;
    KServiceAction mode;
    int renderToken;
    bool cacheRendering : 1;
    bool initialized : 1;
    bool needsConfig : 1;
};

WallpaperRenderThread WallpaperPrivate::s_renderer;
PackageStructure::Ptr WallpaperPrivate::packageStructure(0);

Wallpaper::Wallpaper(QObject *parentObject, const QVariantList &args)
    : d(new WallpaperPrivate(KService::serviceByStorageId(args.count() > 0 ?
                             args[0].toString() : QString()), this))
{
    // now remove first item since those are managed by Wallpaper and subclasses shouldn't
    // need to worry about them. yes, it violates the constness of this var, but it lets us add
    // or remove items later while applets can just pretend that their args always start at 0
    QVariantList &mutableArgs = const_cast<QVariantList &>(args);
    if (!mutableArgs.isEmpty()) {
        mutableArgs.removeFirst();
    }
    setParent(parentObject);

    connect(&WallpaperPrivate::s_renderer, SIGNAL(done(int,QImage,QString,QSize,Wallpaper::ResizeMethod,QColor)),
            this, SLOT(renderCompleted(int,QImage,QString,QSize,Wallpaper::ResizeMethod,QColor)));
}

Wallpaper::~Wallpaper()
{
    delete d;
}

KPluginInfo::List Wallpaper::listWallpaperInfo(const QString &formFactor)
{
    QString constraint;

    if (!formFactor.isEmpty()) {
        constraint.append("[X-Plasma-FormFactors] ~~ '").append(formFactor).append("'");
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Wallpaper", constraint);
    return KPluginInfo::fromServices(offers);
}

Wallpaper *Wallpaper::load(const QString &wallpaperName, const QVariantList &args)
{
    if (wallpaperName.isEmpty()) {
        return 0;
    }

    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(wallpaperName);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Wallpaper", constraint);

    if (offers.isEmpty()) {
        kDebug() << "offers is empty for " << wallpaperName;
        return 0;
    }

    KService::Ptr offer = offers.first();
    KPluginLoader plugin(*offer);

    if (!Plasma::isPluginVersionCompatible(plugin.pluginVersion())) {
        return 0;
    }

    QVariantList allArgs;
    allArgs << offer->storageId() << args;
    QString error;
    Wallpaper *wallpaper = offer->createInstance<Plasma::Wallpaper>(0, allArgs, &error);

    if (!wallpaper) {
        kDebug() << "Couldn't load wallpaper \"" << wallpaperName << "\"! reason given: " << error;
    }

    return wallpaper;
}

Wallpaper *Wallpaper::load(const KPluginInfo &info, const QVariantList &args)
{
    if (!info.isValid()) {
        return 0;
    }
    return load(info.pluginName(), args);
}

PackageStructure::Ptr Wallpaper::packageStructure()
{
    if (!WallpaperPrivate::packageStructure) {
        WallpaperPrivate::packageStructure = new WallpaperPackage();
    }

    return WallpaperPrivate::packageStructure;
}


QString Wallpaper::name() const
{
    if (!d->wallpaperDescription.isValid()) {
        return i18n("Unknown Wallpaper");
    }

    return d->wallpaperDescription.name();
}

QString Wallpaper::icon() const
{
    if (!d->wallpaperDescription.isValid()) {
        return QString();
    }

    return d->wallpaperDescription.icon();
}

QString Wallpaper::pluginName() const
{
    if (!d->wallpaperDescription.isValid()) {
        return QString();
    }

    return d->wallpaperDescription.pluginName();
}

KServiceAction Wallpaper::renderingMode() const
{
    return d->mode;
}

QList<KServiceAction> Wallpaper::listRenderingModes() const
{
    if (!d->wallpaperDescription.isValid()) {
        return QList<KServiceAction>();
    }

    return d->wallpaperDescription.service()->actions();
}

QRectF Wallpaper::boundingRect() const
{
    return d->boundingRect;
}

bool Wallpaper::isInitialized() const
{
    return d->initialized;
}

void Wallpaper::setBoundingRect(const QRectF &boundingRect)
{
    d->boundingRect = boundingRect;
}

void Wallpaper::setRenderingMode(const QString &mode)
{
    if (d->mode.name() == mode) {
        return;
    }

    d->mode = KServiceAction();
    if (!mode.isEmpty()) {
        QList<KServiceAction> modes = listRenderingModes();

        foreach (const KServiceAction &action, modes) {
            if (action.name() == mode) {
                d->mode = action;
                break;
            }
        }
    }
}

void Wallpaper::restore(const KConfigGroup &config)
{
    d->initialized = true;
    init(config);
}

void Wallpaper::init(const KConfigGroup &config)
{
    Q_UNUSED(config);
}

void Wallpaper::save(KConfigGroup &config)
{
    Q_UNUSED(config);
}

QWidget *Wallpaper::createConfigurationInterface(QWidget *parent)
{
    Q_UNUSED(parent);
    return 0;
}

void Wallpaper::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
}

void Wallpaper::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
}

void Wallpaper::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
}

void Wallpaper::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    Q_UNUSED(event)
}

DataEngine *Wallpaper::dataEngine(const QString &name) const
{
    return d->dataEngine(name);
}

bool Wallpaper::configurationRequired() const
{
    return d->needsConfig;
}

void Wallpaper::setConfigurationRequired(bool needsConfig, const QString &reason)
{
    //TODO: implement something for reason. first, we need to decide where/how
    //      to communicate it to the user
    Q_UNUSED(reason)

    if (d->needsConfig == needsConfig) {
        return;
    }

    d->needsConfig = needsConfig;
    emit configurationRequired(needsConfig);
}

bool Wallpaper::isUsingDiskCache() const
{
    return d->cacheRendering;
}

void Wallpaper::setUsingDiskCache(bool useCache)
{
    d->cacheRendering = useCache;
}

void Wallpaper::render(const QString &sourceImagePath, const QSize &size,
                       Wallpaper::ResizeMethod resizeMethod, const QColor &color)
{
    if (sourceImagePath.isEmpty() || !QFile::exists(sourceImagePath)) {
        return;
    }

    if (d->cacheRendering) {
        QString cache = d->cachePath(sourceImagePath, size, resizeMethod, color);
        if (QFile::exists(cache)) {
            kDebug() << "loading cached wallpaper from" << cache;
            QImage img(cache);
            emit renderCompleted(img);
            return;
        }
    }

    d->renderToken = WallpaperPrivate::s_renderer.render(sourceImagePath, size, resizeMethod, color);
}

QString WallpaperPrivate::cachePath(const QString &sourceImagePath, const QSize &size,
                                    Wallpaper::ResizeMethod resizeMethod, const QColor &color) const
{
    const QString id = QString("plasma-wallpapers/%5_%3_%4_%1x%2.png")
                              .arg(size.width()).arg(size.height()).arg(color.name())
                              .arg(resizeMethod).arg(sourceImagePath);
    return KGlobal::dirs()->locateLocal("cache", id);
}

void WallpaperPrivate::renderCompleted(int token, const QImage &image,
                                       const QString &sourceImagePath, const QSize &size,
                                       Wallpaper::ResizeMethod resizeMethod, const QColor &color)
{
    if (token != renderToken) {
        return;
    }

    if (cacheRendering) {
        image.save(cachePath(sourceImagePath, size, resizeMethod, color));
    }

    emit q->renderCompleted(image);
}

} // Plasma namespace

#include "wallpaper.moc"
