/*
 *   Copyright 2007 Matt Broadstone <mbroadst@gmail.com>
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 Riccardo Iaconelli <riccardo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include "corona.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QGraphicsView>
#include <QStringList>

#include <KDebug>
#include <KLocale>
#include <KMimeType>
#include <KWindowSystem>

#include "applet.h"
#include "dataengine.h"
#include "karambamanager.h"
#include "phase.h"
#include "widgets/boxlayout.h"
#include "widgets/icon.h"

using namespace Plasma;

namespace Plasma
{

class Corona::Private
{
public:
    Private()
        : immutable(false),
          formFactor(Planar),
          location(Floating),
          layout(0),
          mimetype("text/x-plasmoidservicename")
    {
    }

    ~Private()
    {
        delete layout;
        qDeleteAll(applets);
    }

    bool immutable;
    Applet::List applets;
    FormFactor formFactor;
    Location location;
    Layout* layout;
    QString mimetype;
};

Corona::Corona(QObject *parent)
    : QGraphicsScene(parent),
      d(new Private)
{
    //setViewport(new QGLWidget(QGLFormat(QGL::StencilBuffer | QGL::AlphaChannel)));
}

Corona::Corona(const QRectF & sceneRect, QObject * parent )
    : QGraphicsScene(sceneRect, parent),
      d(new Private)
{
    //setViewport(new QGLWidget(QGLFormat(QGL::StencilBuffer | QGL::AlphaChannel)));
}

Corona::Corona(qreal x, qreal y, qreal width, qreal height, QObject * parent)
    : QGraphicsScene(x, y, width, height, parent),
      d(new Private)
{
    //setViewport(new QGLWidget(QGLFormat(QGL::StencilBuffer | QGL::AlphaChannel)));
}

Corona::~Corona()
{
    delete d;
}

Location Corona::location() const
{
    return d->location;
}

void Corona::setLocation(Location location)
{
    if (d->location == location) {
        return;
    }

    d->location = location;

    foreach (Applet* applet, d->applets) {
        applet->updateConstraints();
    }
}

FormFactor Corona::formFactor() const
{
    return d->formFactor;
}

void Corona::setFormFactor(FormFactor formFactor)
{
    if (d->formFactor == formFactor) {
        return;
    }

    //kDebug() << "switching FF to " << formFactor;
    d->formFactor = formFactor;
    delete d->layout;
    d->layout = 0;

    switch (d->formFactor) {
        case Planar:
            break;
        case Horizontal:
            d->layout = new BoxLayout(BoxLayout::LeftToRight);
            break;
        case Vertical:
            d->layout = new BoxLayout(BoxLayout::TopToBottom);
            break;
        case MediaCenter:
            break;
        default:
            kDebug() << "This can't be happening!";
            break;
    }

    foreach (Applet* applet, d->applets) {
        applet->updateConstraints();
    }
}

QRectF Corona::maxSizeHint() const
{
    //FIXME: this is a bit of a naive implementation, do you think? =)
    //       we should factor in how much space we actually have left!
    return sceneRect();
}

void Corona::setAppletMimeType(const QString& type)
{
    d->mimetype = type;
}

QString Corona::appletMimeType()
{
    return d->mimetype;
}

void Corona::saveApplets(const QString &config) const
{
    KConfig appletConfig(config);
    foreach (const QString& group, appletConfig.groupList()) {
        appletConfig.deleteGroup(group);
    }

    foreach (Applet *applet, d->applets) {
        KConfigGroup cg(&appletConfig, QString::number(applet->id()));
        //kDebug() << "saving applet " << applet->name();
        cg.writeEntry("plugin", applet->pluginName());
        cg.writeEntry("geometry", QRect(applet->pos().toPoint(), applet->boundingRect().size().toSize()));
    }
}

void Corona::saveApplets() const
{
    saveApplets("plasma-appletsrc");
}

void Corona::loadApplets(const QString& config)
{
    clearApplets();

    KConfig appletConfig(config, KConfig::OnlyLocal);
    foreach (const QString& group, appletConfig.groupList()) {
        KConfigGroup cg(&appletConfig, group);
        addApplet(cg.readEntry("plugin", QString()), QVariantList(),
                  group.toUInt(), cg.readEntry("geometry", QRectF()));
    }
}

void Corona::loadApplets()
{
    loadApplets("plasma-appletsrc");
}

void Corona::clearApplets()
{
    qDeleteAll(d->applets);
    d->applets.clear();
}

Applet* Corona::addApplet(const QString& name, const QVariantList& args, uint id, const QRectF& geometry)
{
    Applet* applet = Applet::loadApplet(name, id, args);
    if (!applet) {
        kDebug() << "Applet " << name << " could not be loaded.";
        applet = new Applet;
        applet->setFailedToLaunch(true);
    }

    qreal appWidth = applet->boundingRect().width();
    qreal appHeight = applet->boundingRect().height();

    if (geometry.isValid()) {
        applet->setGeometry(geometry);
    } else { 
        applet->setGeometry( QRectF(geometry.topLeft(),applet->sizeHint()) );
    }

#if 0
    if (!geometry.size().isValid()) {
        
    }

    if (geometry.isValid()) {
        applet->setGeometry(geometry);
    } else if (geometry.x() != -1 && geometry.y() != -1) {
        // yes, this means we can't have items start -1, -1
        applet->setPos(geometry.topLeft() - QPointF(applet->boundingRect().width()/2,
                                                   applet->boundingRect().height()/2));
    } else {
        //TODO: Make sure new applets don't overlap with existing ones
        // Center exactly:
        applet->setPos((width() / 2) - (appWidth / 2), (height() / 2) - (appHeight / 2));
    }
#endif

    addItem(applet);
    applet->updateConstraints();

    //applet->constraintsUpdated();
    d->applets << applet;
    connect(applet, SIGNAL(destroyed(QObject*)),
            this, SLOT(appletDestroyed(QObject*)));
    Phase::self()->animateItem(applet, Phase::Appear);

    return applet;
}

void Corona::addKaramba(const KUrl& path)
{
    QGraphicsItemGroup* karamba = KarambaManager::loadKaramba(path, this);
    if (karamba) {
        addItem(karamba);
        Phase::self()->animateItem(karamba, Phase::Appear);
    } else {
        kDebug() << "Karamba " << path << " could not be loaded.";
    }
}

void Corona::dragEnterEvent( QGraphicsSceneDragDropEvent *event)
{
    kDebug() << "Corona::dragEnterEvent(QGraphicsSceneDragDropEvent* event)";
    if (event->mimeData()->hasFormat(d->mimetype) ||
        KUrl::List::canDecode(event->mimeData())) {
        event->acceptProposedAction();
        //TODO Create the applet, move to mouse position then send the 
        //     following event to lock it to the mouse
        //QMouseEvent event(QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, event->mouseButtons(), 0);
        //QApplication::sendEvent(this, &event);
    }

    event->accept();
    //TODO Allow dragging an applet from another Corona into this one while
    //     keeping its settings etc.
}

void Corona::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
   // kDebug() << "Corona::dragLeaveEvent(QGraphicsSceneDragDropEvent* event)";
    //TODO If an established Applet is dragged out of the Corona, remove it and
    //     create a QDrag type thing to keep the Applet's settings

    QGraphicsScene::dragLeaveEvent(event);
}

void Corona::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    QGraphicsScene::dragMoveEvent(event);

    event->accept();
    //kDebug() << "Corona::dragMoveEvent(QDragMoveEvent* event)";
}

void Corona::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (itemAt(event->scenePos())) {
        QGraphicsScene::dropEvent(event);
        return;
    }

    //kDebug() << "Corona::dropEvent(QDropEvent* event)";
    if (event->mimeData()->hasFormat(d->mimetype)) {
        QString plasmoidName;
        plasmoidName = event->mimeData()->data(d->mimetype);
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
    } else {
        QGraphicsScene::dropEvent(event);
    }
}

// void Corona::contextMenuEvent(QGraphicsSceneContextMenuEvent *contextMenuEvent)
// {
// }

void Corona::appletDestroyed(QObject* object)
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

bool Corona::isImmutable() const
{
    return d->immutable;
}

void Corona::setImmutable(bool immutable)
{
    if (d->immutable == immutable) {
        return;
    }

    d->immutable = immutable;
    foreach (QGraphicsItem* item, items()) {
        QGraphicsItem::GraphicsItemFlags flags = item->flags();
        if (immutable) {
            flags ^= QGraphicsItem::ItemIsMovable;
        } else {
            flags |= QGraphicsItem::ItemIsMovable;
        }
        item->setFlags(flags);
    }
}

} // namespace Plasma

#include "corona.moc"

