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

#include "config-plasma.h"

#include <QColor>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QAction>
#include <QQueue>
#include <QTimer>
#include <QRunnable>
#include <QThreadPool>

#include <kdebug.h>
#include <kglobal.h>
#include <kservicetypetrader.h>
#include <kstandarddirs.h>

#ifndef PLASMA_NO_KIO
#include <kio/job.h>
#endif

#ifndef PLASMA_NO_SOLID
#include <solid/device.h>
#include <solid/deviceinterface.h>
#endif

#include <version.h>

#include "plasma/package.h"
#include "plasma/private/dataengineconsumer_p.h"
#include "plasma/private/packages_p.h"
#include "plasma/private/wallpaper_p.h"

namespace Plasma
{

class SaveImageThread : public QRunnable
{
    QImage m_image;
    QString m_filePath;

    public:
    SaveImageThread(const QImage &image, const QString &filePath)
    {
        m_image = image;
        m_filePath = filePath;
    }

    void run()
    {
        m_image.save(m_filePath);
    }
};
    
LoadImageThread::LoadImageThread(const QString &filePath)
{
    m_filePath = filePath;
}

void LoadImageThread::run()
{
    QImage image;
    image.load(m_filePath);
    emit done(image);
}

class WallpaperWithPaint : public Wallpaper
{
public:
    WallpaperWithPaint(QObject *parent, const QVariantList &args)
        : Wallpaper(parent, args)
    {
    }

    virtual void paint(QPainter *painter, const QRectF &exposedRect)
    {
        if (d->script) {
            d->script->paint(painter, exposedRect);
        }
    }
};

QList<WallpaperRenderThread *> WallpaperPrivate::s_renderers;
QQueue<WallpaperPrivate::RenderRequest> WallpaperPrivate::s_renderQueue;
PackageStructure::Ptr WallpaperPrivate::s_packageStructure(0);

Wallpaper::Wallpaper(QObject * parentObject)
    : d(new WallpaperPrivate(KService::serviceByStorageId(QString()), this))
{
    setParent(parentObject);
}

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

KPluginInfo::List Wallpaper::listWallpaperInfoForMimetype(const QString &mimetype, const QString &formFactor)
{
    QString constraint = QString("'%1' in [X-Plasma-DropMimeTypes]").arg(mimetype);
    if (!formFactor.isEmpty()) {
        constraint.append("[X-Plasma-FormFactors] ~~ '").append(formFactor).append("'");
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Wallpaper", constraint);
    kDebug() << offers.count() << constraint;
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
    QVariantList allArgs;
    allArgs << offer->storageId() << args;

    if (!offer->property("X-Plasma-API").toString().isEmpty()) {
        kDebug() << "we have a script using the"
                 << offer->property("X-Plasma-API").toString() << "API";
        return new WallpaperWithPaint(0, allArgs);
    }

    KPluginLoader plugin(*offer);

    if (!Plasma::isPluginVersionCompatible(plugin.pluginVersion())) {
        return 0;
    }

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

PackageStructure::Ptr Wallpaper::packageStructure(Wallpaper *paper)
{
    if (paper) {
        PackageStructure::Ptr package(new WallpaperPackage(paper));
        return package;
    }

    if (!WallpaperPrivate::s_packageStructure) {
        WallpaperPrivate::s_packageStructure = new WallpaperPackage();
    }

    return WallpaperPrivate::s_packageStructure;
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
    QSizeF oldBoundingRectSize = d->boundingRect.size();
    d->boundingRect = boundingRect;

    if (!d->targetSize.isValid() || d->targetSize == oldBoundingRectSize)  {
        d->targetSize = boundingRect.size();
        emit renderHintsChanged();
    }
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
    init(config);
    d->initialized = true;
}

void Wallpaper::init(const KConfigGroup &config)
{
    if (d->script) {
        d->initScript();
        d->script->initWallpaper(config);
    }
}

void Wallpaper::save(KConfigGroup &config)
{
    if (d->script) {
        d->script->save(config);
    }
}

QWidget *Wallpaper::createConfigurationInterface(QWidget *parent)
{
    if (d->script) {
        return d->script->createConfigurationInterface(parent);
    } else {
        return 0;
    }
}

void Wallpaper::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (d->script) {
        return d->script->mouseMoveEvent(event);
    }
}

void Wallpaper::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (d->script) {
        return d->script->mousePressEvent(event);
    }
}

void Wallpaper::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (d->script) {
        return d->script->mouseReleaseEvent(event);
    }
}

void Wallpaper::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (d->script) {
        return d->script->wheelEvent(event);
    }
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

bool Wallpaper::isUsingRenderingCache() const
{
    return d->cacheRendering;
}

void Wallpaper::setUsingRenderingCache(bool useCache)
{
    d->cacheRendering = useCache;
}

void Wallpaper::setResizeMethodHint(Wallpaper::ResizeMethod resizeMethod)
{
    d->lastResizeMethod = resizeMethod;
    emit renderHintsChanged();
}

void Wallpaper::setTargetSizeHint(const QSizeF &targetSize)
{
    d->targetSize = targetSize;
    emit renderHintsChanged();
}

void Wallpaper::render(const QString &sourceImagePath, const QSize &size,
                       Wallpaper::ResizeMethod resizeMethod, const QColor &color)
{
    if (sourceImagePath.isEmpty() || !QFile::exists(sourceImagePath)) {
        //kDebug() << "failed on:" << sourceImagePath;
        return;
    }

    if (d->lastResizeMethod != resizeMethod) {
        d->lastResizeMethod = resizeMethod;
        emit renderHintsChanged();
    }

    if (d->cacheRendering) {
        QFileInfo info(sourceImagePath);
        QString cache = d->cacheKey(sourceImagePath, size, resizeMethod, color);
        if (d->findInCache(cache, info.lastModified().toTime_t())) {
            return;
        }
    }

    for (int i = 0; i < WallpaperPrivate::s_renderers.size(); i++) {
        if (d->renderToken == WallpaperPrivate::s_renderers[i]->currentToken()) {
            d->renderToken = WallpaperPrivate::s_renderers[i]->render(sourceImagePath, size, resizeMethod, color);
            return;
        }
    }

#ifndef PLASMA_NO_SOLID
    const int numProcs = Solid::Device::listFromType(Solid::DeviceInterface::Processor).count();
#else
    const int numProcs = 1;
#endif
    if (WallpaperPrivate::s_renderers.size() < qMax(numProcs, 1)) {
        WallpaperRenderThread *renderThread = new WallpaperRenderThread();
        WallpaperPrivate::s_renderers.append(renderThread);
        d->renderToken = renderThread->render(sourceImagePath, size, resizeMethod, color);
        connect(renderThread, SIGNAL(done(WallpaperRenderThread*,int,QImage,QString,QSize,int,QColor)),
                this, SLOT(renderCompleted(WallpaperRenderThread*,int,QImage,QString,QSize,int,QColor)), Qt::UniqueConnection);
    } else {
         WallpaperPrivate::RenderRequest request;
         request.parent = this;
         request.file = sourceImagePath;
         request.size = size;
         request.resizeMethod = resizeMethod;
         request.color = color;

         for (int i = 0; i < WallpaperPrivate::s_renderQueue.size(); i++) {
            if (WallpaperPrivate::s_renderQueue[i].parent.data() == this){
                WallpaperPrivate::s_renderQueue[i] = request;
                return;
            }
         }
         WallpaperPrivate::s_renderQueue.append(request);
    }
    //kDebug() << "rendering" << sourceImagePath << ", token is" << d->renderToken;
}

WallpaperPrivate::WallpaperPrivate(KService::Ptr service, Wallpaper *wallpaper) :
    q(wallpaper),
    wallpaperDescription(service),
    package(0),
    renderToken(-1),
    lastResizeMethod(Wallpaper::ScaledResize),
    script(0),
    cacheRendering(false),
    initialized(false),
    needsConfig(false),
    scriptInitialized(false),
    previewing(false)
{
    if (wallpaperDescription.isValid()) {
        QString api = wallpaperDescription.property("X-Plasma-API").toString();

        if (!api.isEmpty()) {
            const QString path = KStandardDirs::locate("data",
                    "plasma/wallpapers/" + wallpaperDescription.pluginName() + '/');
            PackageStructure::Ptr structure =
                Plasma::packageStructure(api, Plasma::WallpaperComponent);
            structure->setPath(path);
            package = new Package(path, structure);

            script = Plasma::loadScriptEngine(api, q);
            if (!script) {
                kDebug() << "Could not create a" << api << "ScriptEngine for the"
                        << wallpaperDescription.name() << "Wallpaper.";
                delete package;
                package = 0;
            }
       }
    }
}

QString WallpaperPrivate::cacheKey(const QString &sourceImagePath, const QSize &size,
                                   int resizeMethod, const QColor &color) const
{
    const QString id = QString("%5_%3_%4_%1x%2")
                              .arg(size.width()).arg(size.height()).arg(color.name())
                              .arg(resizeMethod).arg(sourceImagePath);
    return id;
}

QString WallpaperPrivate::cachePath(const QString &key) const
{
    return KGlobal::dirs()->locateLocal("cache", "plasma-wallpapers/" + key + ".png");
}

void WallpaperPrivate::renderCompleted(WallpaperRenderThread *currentRenderer, int token, const QImage &image,
                                       const QString &sourceImagePath, const QSize &size,
                                       int resizeMethod, const QColor &color)
{
    q->disconnect(currentRenderer, SIGNAL(done(WallpaperRenderThread*,int,QImage,QString,QSize,int,QColor)),
                    q, SLOT(renderCompleted(WallpaperRenderThread*,int,QImage,QString,QSize,int,QColor)));

    if (!s_renderQueue.isEmpty()) {
        while (!s_renderQueue.isEmpty()) {
            WallpaperPrivate::RenderRequest request = s_renderQueue.dequeue();

            if (!request.parent) {
                continue;
            }

            currentRenderer->wait();
            request.parent.data()->d->renderToken = currentRenderer->render(request.file, request.size, request.resizeMethod, request.color);
            QObject::connect(currentRenderer, SIGNAL(done(WallpaperRenderThread*,int,QImage,QString,QSize,int,QColor)),
                    request.parent.data(), SLOT(renderCompleted(WallpaperRenderThread*,int,QImage,QString,QSize,int,QColor)), Qt::UniqueConnection);

            break;
        }
    } else {
         for (int i = 0; i < s_renderers.size(); i++) {
            if (s_renderers[i] == currentRenderer){
                s_renderers.removeAt(i);
            }
         }
         currentRenderer->deleteLater();
         currentRenderer = 0;
    }

    if (token != renderToken) {
        //kDebug() << "render token mismatch" << token << renderToken;
        return;
    }

    if (cacheRendering) {
        q->insertIntoCache(cacheKey(sourceImagePath, size, resizeMethod, color), image);
    }

    //kDebug() << "rendering complete!";
    emit q->renderCompleted(image);
}

// put all setup routines for script here. at this point we can assume that
// package exists and that we have a script engine
void WallpaperPrivate::setupScriptSupport()
{
    Q_ASSERT(package);
    kDebug() << "setting up script support, package is in" << package->path()
             << "which is a" << package->structure()->type() << "package"
             << ", main script is" << package->filePath("mainscript");

    QString translationsPath = package->filePath("translations");
    if (!translationsPath.isEmpty()) {
        //FIXME: we should _probably_ use a KComponentData to segregate the applets
        //       from each other; but I want to get the basics working first :)
        KGlobal::dirs()->addResourceDir("locale", translationsPath);
        KGlobal::locale()->insertCatalog(package->metadata().pluginName());
    }
}

void WallpaperPrivate::initScript()
{
    if (script && !scriptInitialized) {
        setupScriptSupport();
        script->init();
        scriptInitialized = true;
    }
}

bool WallpaperPrivate::findInCache(const QString &key, unsigned int lastModified)
{
    if (cacheRendering) {
        QString cache = cachePath(key);
        if (QFile::exists(cache)) {
            if (lastModified > 0) {
                QFileInfo info(cache);
                if (info.lastModified().toTime_t() < lastModified) {
                    return false;
                }
            }

            LoadImageThread *loadImageT = new LoadImageThread(cache);
            q->connect(loadImageT, SIGNAL(done(const QImage&)), q, SIGNAL(renderCompleted(const QImage&)));
            QThreadPool::globalInstance()->start(loadImageT);

            return true;
        }
    }

    return false;
}

bool Wallpaper::findInCache(const QString &key, QImage &image, unsigned int lastModified)
{
    if (d->cacheRendering) {
        QString cache = d->cachePath(key);
        if (QFile::exists(cache)) {
            if (lastModified > 0) {
                QFileInfo info(cache);
                if (info.lastModified().toTime_t() < lastModified) {
                    return false;
                }
            }

            image.load(cache);
            return true;
        }
    }

    return false;
}

void Wallpaper::insertIntoCache(const QString& key, const QImage &image)
{
    //TODO: cache limits?
    if (key.isEmpty()) {
        return;
    }

    if (d->cacheRendering) {
        if (image.isNull()) {
#ifndef PLASMA_NO_KIO
            KIO::file_delete(d->cachePath(key));
#else
            QFile f(d->cachePath(key));
            f.remove():
#endif
        } else {
            QThreadPool::globalInstance()->start(new SaveImageThread(image, d->cachePath(key)));
        }
    }
}

QList<QAction*> Wallpaper::contextualActions() const
{
    return contextActions;
}

void Wallpaper::setContextualActions(const QList<QAction*> &actions)
{
    contextActions = actions;
}

bool Wallpaper::isPreviewing() const
{
    return d->previewing;
}

void Wallpaper::setPreviewing(bool previewing)
{
    d->previewing = previewing;
}

const Package *Wallpaper::package() const
{
    return d->package;
}

} // Plasma namespace

#include "wallpaper.moc"
#include "private/wallpaper_p.moc"
