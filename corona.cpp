/*
 *   Copyright (C) 2007 Matt Broadstone <mbroadst@gmail.com>
 *   Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright (C) 2007 Riccardo Iaconelli <riccardo@kde.org>
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
#include "widgets/vboxlayout.h"
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

Corona::Corona(QObject * parent)
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
        applet->constraintsUpdated();
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
            //d->layout = new HBoxLayout;
            break;
        case Vertical:
            d->layout = new VBoxLayout;
            break;
        case MediaCenter:
            break;
        default:
            kDebug() << "This can't be happening!";
            break;
    }

    foreach (Applet* applet, d->applets) {
        applet->constraintsUpdated();
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

Applet* Corona::addApplet(const QString& name, const QStringList& args)
{
    Applet* applet = Applet::loadApplet(name, 0, args);
    if (!applet) {
        kDebug() << "Applet " << name << " could not be loaded.";
        applet = new Applet;
        applet->setFailedToLaunch(true);
    }

    qreal appWidth = applet->boundingRect().width();
    qreal appHeight = applet->boundingRect().height();
    //TODO: Make sure new applets don't overlap with existing ones
    // Center exactly:
    applet->setPos((width() / 2) - (appWidth / 2),(height() / 2) - (appHeight / 2));
    addItem(applet);
    
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
    if (event->mimeData()->hasFormat("text/x-plasmoidservicename") ||
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
}

void Corona::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    QGraphicsScene::dragMoveEvent(event);

    event->accept();
    //kDebug() << "Corona::dragMoveEvent(QDragMoveEvent* event)";
}

void Corona::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    //kDebug() << "Corona::dropEvent(QDropEvent* event)";
    if (event->mimeData()->hasFormat("text/x-plasmoidservicename")) {
        //TODO This will pretty much move into dragEnterEvent()
        QString plasmoidName;
        plasmoidName = event->mimeData()->data("text/x-plasmoidservicename");
        addApplet(plasmoidName);
        Applet *applet = d->applets.last();
        // TODO: should we place it centered on the mouse cursor?  If so, the
        // default "zoom in" animation needs changing to zoom in from the same
        // point, or it just looks odd
        applet->setPos(event->scenePos()); // -
        //        QPoint(applet->boundingRect().width()/2,applet->boundingRect().height()/2));

        event->acceptProposedAction();
    } else if (KUrl::List::canDecode(event->mimeData())) {
        KUrl::List urls = KUrl::List::fromMimeData(event->mimeData());	
        foreach (const KUrl& url, urls) {
            KMimeType::Ptr mime = KMimeType::findByUrl(url);
            QString mimeName = mime->name();
//             kDebug() << mimeName;
            KPluginInfo::List appletList = Applet::knownAppletsForMimetype(mimeName);


            //FIXME: we should probably show a dialog here to choose which plasmoid load.
            addApplet(appletList.first().pluginName());

            QStringList args;
            args << url.url();
            Applet* button = addApplet("url", args);
            if (button) {
//                 button->setSize(128,128);
                button->setPos(event->scenePos() - QPoint(button->boundingRect().width()/2,
                               button->boundingRect().height()/2));
            }
            addItem(button);
        }
        event->acceptProposedAction();
    } else
        QGraphicsScene::dropEvent(event);
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

