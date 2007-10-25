/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
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

#include "containment.h"

#include <QAction>
#include <QDesktopWidget>
#include <QFile>
#include <QGraphicsSceneContextMenuEvent>
#include <QMimeData>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include <KApplication>
#include <KAuthorized>
#include <KIcon>
#include <KMenu>
#include <KMimeType>
#include <KRun>
#include <KServiceTypeTrader>
#include <KStandardDirs>

#include "corona.h"
#include "phase.h"
#include "svg.h"

#include "widgets/freelayout.h"
#include "widgets/boxlayout.h"

namespace Plasma
{

static const int INTER_CONTAINMENT_MARGIN = 6;

class Containment::Private
{
public:
    Private()
        : formFactor(Planar),
          location(Floating),
          layout(0),
          background(0),
          bitmapBackground(0),
          screen(0),
          immutable(false)
    {
    }

    ~Private()
    {
        qDeleteAll(applets);
        applets.clear();
        delete layout;
        delete bitmapBackground;
    }

    FormFactor formFactor;
    Location location;
    Layout* layout;
    Applet::List applets;
    Plasma::Svg *background;
    QPixmap* bitmapBackground;
    QString wallpaperPath;
    QSize size;
    int screen;
    bool immutable;
};

Containment::Containment(QGraphicsItem* parent,
                         const QString& serviceId,
                         uint containmentId)
    : Applet(parent, serviceId, containmentId),
      d(new Private)
{
}

Containment::Containment(QObject* parent, const QVariantList& args)
    : Applet(parent, args),
      d(new Private)
{
}

Containment::~Containment()
{
    delete d;
}

void Containment::init()
{
    setCachePaintMode(NoCacheMode);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, false);
    setAcceptDrops(true);

    if (type() == DesktopContainment) {
        KConfigGroup config(KGlobal::config(), "General");
        d->wallpaperPath = config.readEntry("wallpaper", KStandardDirs::locate("wallpaper", "plasma-default.png"));

        kDebug() << "wallpaperPath is" << d->wallpaperPath << QFile::exists(d->wallpaperPath);
        if (d->wallpaperPath.isEmpty() ||
            !QFile::exists(d->wallpaperPath)) {
            kDebug() << "SVG wallpaper!";
            d->background = new Plasma::Svg("widgets/wallpaper", this);
        }
    }

    //TODO: would be nice to not do this on init, as it causes Phase to init
    connect(Phase::self(), SIGNAL(animationComplete(QGraphicsItem*,Plasma::Phase::Animation)),
            this, SLOT(appletDisappearComplete(QGraphicsItem*,Plasma::Phase::Animation)));
}

void Containment::initConstraints(KConfigGroup* group)
{
    //kDebug() << "initConstraints" << group->name() << type();
    setLocation((Plasma::Location)group->readEntry("location", (int)Plasma::Desktop));
    setGeometry(group->readEntry("geometry", QRectF()));
    setFormFactor((Plasma::FormFactor)group->readEntry("formfactor", (int)Plasma::Planar));
    setScreen(group->readEntry("screen", 0));
}

void Containment::saveConstraints(KConfigGroup* group) const
{
    group->writeEntry("screen", d->screen);
    group->writeEntry("formfactor", (int)d->formFactor);
    group->writeEntry("location", (int)d->location);
}

Containment::Type Containment::type()
{
    return DesktopContainment;
}

void Containment::paintInterface(QPainter *painter,
                                 const QStyleOptionGraphicsItem *option,
                                 const QRect& contentsRect)
{
    //FIXME: this should probably ALL move to the Desktop containment, save for drawing a rect
    //       in case there is no other drawing going on
    if (type() != DesktopContainment) {
        return;
    }

    //kDebug() << "paintInterface of background";
    //TODO: we should have a way to do this outside of the paint event!
    if (d->background) {
        d->background->resize(contentsRect.size());
    } else if (!d->wallpaperPath.isEmpty()) {
        if (!d->bitmapBackground || !(d->bitmapBackground->size() == contentsRect.size())) {
            delete d->bitmapBackground;
            d->bitmapBackground = new QPixmap(d->wallpaperPath);
            (*d->bitmapBackground) = d->bitmapBackground->scaled(contentsRect.size());
        }
    } else {
        // got nothing to paint!
        //kDebug() << "got nothing?";
        painter->drawRect(contentsRect.adjusted(1, 1, -1, -1));
        return;
    }

    // draw the background untransformed (saves lots of per-pixel-math)
    painter->save();
    painter->resetTransform();

    // blit the background (saves all the per-pixel-products that blending does)
    painter->setCompositionMode(QPainter::CompositionMode_Source);

    if (d->background) {
        // Plasma::Svg doesn't support drawing only part of the image (it only
        // supports drawing the whole image to a rect), so we blit to 0,0-w,h
        d->background->paint(painter, contentsRect);
    //kDebug() << "draw svg of background";
    } else if (d->bitmapBackground) {
        // for pixmaps we draw only the exposed part (untransformed since the
        // bitmapBackground already has the size of the viewport)
        painter->drawPixmap(option->exposedRect, *d->bitmapBackground, option->exposedRect);
    //kDebug() << "draw pixmap of background";
    }

    // restore transformation and composition mode
    painter->restore();
}

QSizeF Containment::contentSizeHint() const
{
    return d->size;
}

void Containment::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    //kDebug() << "let's see if we manage to get a context menu here, huh";
    if (!scene() || !KAuthorized::authorizeKAction("desktop_contextmenu")) {
        QGraphicsItem::contextMenuEvent(event);
        return;
    }

    QPointF point = event->scenePos();
    QGraphicsItem* item = scene()->itemAt(point);
    if (item == this) {
        item = 0;
    }

    Applet* applet = 0;

    while (item) {
        applet = qgraphicsitem_cast<Applet*>(item);
        if (applet) {
            break;
        }

        item = item->parentItem();
    }

    KMenu desktopMenu;
    //kDebug() << "context menu event " << immutable;
    if (!applet) {
        if (!scene() || static_cast<Corona*>(scene())->isImmutable()) {
            QGraphicsItem::contextMenuEvent(event);
            return;
        }

        //FIXME: change this to show this only in debug mode (or not at all?)
        //       before final release
        QList<QAction*> actions = contextActions();

        if (actions.count() < 1) {
            QGraphicsItem::contextMenuEvent(event);
            return;
        }

        foreach(QAction* action, actions) {
            desktopMenu.addAction(action);
        }
    } else if (applet->isImmutable()) {
        QGraphicsItem::contextMenuEvent(event);
        return;
    } else {
        bool hasEntries = false;
        if (applet->hasConfigurationInterface()) {
            QAction* configureApplet = new QAction(i18n("%1 Settings...", applet->name()), &desktopMenu);
            connect(configureApplet, SIGNAL(triggered(bool)),
                    applet, SLOT(showConfigurationInterface()));
            desktopMenu.addAction(configureApplet);
            hasEntries = true;
        }

        if (scene() && !static_cast<Corona*>(scene())->isImmutable()) {
            QAction* closeApplet = new QAction(i18n("Remove this %1", applet->name()), &desktopMenu);
            QVariant appletV;
            appletV.setValue((QObject*)applet);
            closeApplet->setData(appletV);
            connect(closeApplet, SIGNAL(triggered(bool)),
                    this, SLOT(destroyApplet()));
            desktopMenu.addAction(closeApplet);
            hasEntries = true;
        }

        QList<QAction*> actions = applet->contextActions();
        if (!actions.isEmpty()) {
            desktopMenu.addSeparator();
            foreach(QAction* action, actions) {
                desktopMenu.addAction(action);
            }
            hasEntries = true;
        }

        if (!hasEntries) {
            QGraphicsItem::contextMenuEvent(event);
            return;
        }
    }

    event->accept();
    desktopMenu.exec(event->screenPos());
}

void Containment::setFormFactor(FormFactor formFactor)
{
    if (d->formFactor == formFactor && d->layout) {
        return;
    }

//kDebug() << "switching FF to " << formFactor;
    d->formFactor = formFactor;
    delete d->layout;
    d->layout = 0;

    switch (d->formFactor) {
        case Planar:
            d->layout = new FreeLayout(this);
            break;
        case Horizontal:
            d->layout = new BoxLayout(BoxLayout::LeftToRight, this);
            d->layout->setMargin(0);
            d->layout->setSpacing(0);
            break;
        case Vertical:
            d->layout = new BoxLayout(BoxLayout::TopToBottom, this);
            d->layout->setMargin(0);
            d->layout->setSpacing(0);
            break;
        case MediaCenter:
            //FIXME: need a layout type here!
            break;
        default:
            kDebug() << "This can't be happening! Or... can it? ;)";
            break;
    }

    foreach (Applet* applet, d->applets) {
        d->layout->addItem(applet);
        applet->updateConstraints(Plasma::FormFactorConstraint);
    }

    updateConstraints(Plasma::FormFactorConstraint);
}

FormFactor Containment::formFactor() const
{
    return d->formFactor;
}

void Containment::setLocation(Location location)
{
    if (d->location == location) {
        return;
    }

    d->location = location;

    foreach (Applet* applet, d->applets) {
        applet->updateConstraints(Plasma::LocationConstraint);
    }

    updateConstraints(Plasma::LocationConstraint);
}

Location Containment::location() const
{
    return d->location;
}

void Containment::clearApplets()
{
    qDeleteAll(d->applets);
    d->applets.clear();
}

Applet* Containment::addApplet(const QString& name, const QVariantList& args, uint id, const QRectF& geometry, bool delayInit)
{
    Applet* applet = Applet::loadApplet(name, id, args);
    if (!applet) {
        kDebug() << "Applet" << name << "could not be loaded.";
        applet = new Applet;
    }

    applet->setParentItem(this);
    //panels don't want backgrounds, which is important when setting geometry
    if (type() == PanelContainment) {
        applet->setDrawStandardBackground(false);
    }
    //the applet needs to be given constraints before it can set its geometry
    applet->updateConstraints(Plasma::AllConstraints);

    //kDebug() << "adding applet" << applet->name() << "with a default geometry of" << geometry << geometry.isValid();
    if (geometry.isValid()) {
        applet->setGeometry(geometry);
    } else if (geometry.x() != -1 && geometry.y() != -1) {
        // yes, this means we can't have items start -1, -1
        applet->setGeometry(QRectF(geometry.topLeft() - QPointF(applet->sizeHint().width()/2,
                                                                applet->sizeHint().height()/2),
                                   applet->sizeHint()));
    } else {
        //TODO: Make sure new applets don't overlap with existing ones
        // Center exactly:
        QSizeF size = applet->sizeHint();
        qreal appletWidth = size.width();
        qreal appletHeight = size.height();
        qreal width = this->geometry().width();
        qreal height = this->geometry().height();
        //kDebug() << "measuring geometry with" << appletWidth << appletHeight << width << height;
        applet->setGeometry(QRectF(QPointF((width / 2) - (appletWidth / 2), (height / 2) - (appletHeight / 2)), size));
    }

    if (d->layout) {
        d->layout->addItem(applet);
    }

    kDebug() << applet->name() << "sizehint:" << applet->sizeHint()
                               << "geometry:" << applet->geometry();

    if (!delayInit) {
        applet->init();
    }

    d->applets << applet;
    connect(applet, SIGNAL(destroyed(QObject*)),
            this, SLOT(appletDestroyed(QObject*)));
    Phase::self()->animateItem(applet, Phase::Appear);

    return applet;
}

void Containment::appletDestroyed(QObject* object)
{
    // we do a static_cast here since it really isn't an Applet by this
    // point anymore since we are in the qobject dtor. we don't actually
    // try and do anything with it, we just need the value of the pointer
    // so this unsafe looking code is actually just fine.
    Applet* applet = static_cast<Plasma::Applet*>(object);
    int index = d->applets.indexOf(applet);

    if (index > -1) {
        d->applets.removeAt(index);
    }
}

void Containment::destroyApplet()
{
    QAction *action = qobject_cast<QAction*>(sender());

    if (!action) {
        return;
    }

    Applet *applet = qobject_cast<Applet*>(action->data().value<QObject*>());
    Phase::self()->animateItem(applet, Phase::Disappear);
}

void Containment::appletDisappearComplete(QGraphicsItem *item, Plasma::Phase::Animation anim)
{
    if (anim == Phase::Disappear) {
        if (item->parentItem() == this) {
            Applet *applet = qgraphicsitem_cast<Applet*>(item);

            if (applet) {
                applet->destroy();
            }
        }
    }
}

Applet::List Containment::applets() const
{
    return d->applets;
}

void Containment::setScreen(int screen)
{
    // screen of -1 means no associated screen.

    //kDebug() << "setting screen to" << screen;
    QDesktopWidget desktop;
    int numScreens = desktop.numScreens();
    if (screen < -1 || screen > numScreens - 1) {
        screen = -1;
    }

    kDebug() << "APAKU: setting screen to " << screen << "and type is" << type();
    if (screen > -1) {
        if (type() == DesktopContainment) {
            setGeometry(desktop.screenGeometry(screen));
            //kDebug() << "setting geometry to" << desktop.screenGeometry(screen) << geometry();
        } else if (type() == PanelContainment) {
            QDesktopWidget desktop;
            QRect r = desktop.screenGeometry(screen);
            kDebug() << "APAKU: we are a panel, let's move ourselves to a negative coordinate system" << r;
            //FIXME PANELS: multiple panel support means having to move the panels up
            //              this requires a proper panel manager, discuss in the panel
            //              irc meeting
            translate(0, -r.height() - INTER_CONTAINMENT_MARGIN);
        }
    }

    d->screen = screen;
    updateConstraints(Plasma::ScreenConstraint);
}

int Containment::screen() const
{
    return d->screen;
}

KPluginInfo::List Containment::knownContainments(const QString &category,
                                                 const QString &parentApp)
{
    QString constraint;

    if (parentApp.isEmpty()) {
        constraint.append("not exist [X-KDE-ParentApp]");
    } else {
        constraint.append("[X-KDE-ParentApp] == '").append(parentApp).append("'");
    }

    if (!category.isEmpty()) {
        if (!constraint.isEmpty()) {
            constraint.append(" and ");
        }

        constraint.append("[X-KDE-PluginInfo-Category] == '").append(category).append("'");
        if (category == "Miscellaneous") {
            constraint.append(" or (not exist [X-KDE-PluginInfo-Category] or [X-KDE-PluginInfo-Category] == '')");
        }
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Containment", constraint);
    //kDebug() << "constraint was" << constraint << "which got us" << offers.count() << "matches";
    return KPluginInfo::fromServices(offers);
}

KPluginInfo::List Containment::knownContainmentsForMimetype(const QString &mimetype)
{
    QString constraint = QString("'%1' in MimeTypes").arg(mimetype);
    //kDebug() << "knownContainmentsForMimetype with" << mimetype << constraint;
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Containment", constraint);
    return KPluginInfo::fromServices(offers);
}

void Containment::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    //kDebug() << "drop event:" << event->mimeData()->text();

    QString mimetype(static_cast<Corona*>(scene())->appletMimeType());

    if (event->mimeData()->hasFormat(mimetype) && scene()) {
        QString plasmoidName;
        plasmoidName = event->mimeData()->data(mimetype);
        QRectF geom(event->scenePos(), QSize(0, 0));
        addApplet(plasmoidName, QVariantList(), 0, geom);
        event->acceptProposedAction();
    } else if (KUrl::List::canDecode(event->mimeData())) {
        KUrl::List urls = KUrl::List::fromMimeData(event->mimeData());
        foreach (const KUrl& url, urls) {
            KMimeType::Ptr mime = KMimeType::findByUrl(url);
            QString mimeName = mime->name();
            QRectF geom(event->scenePos(), QSize(0, 0));
            QVariantList args;
            args << url.url();
//             kDebug() << mimeName;
            KPluginInfo::List appletList = Applet::knownAppletsForMimetype(mimeName);

            if (appletList.isEmpty()) {
                // no special applet associated with this mimetype, let's
                addApplet("url", args, 0, geom);
            } else {
                //TODO: should we show a dialog here to choose which plasmoid load if
                //appletList.count() > 0?
                addApplet(appletList.first().pluginName(), args, 0, geom);
            }
        }
        event->acceptProposedAction();
    }
}

}

#include "containment.moc"
