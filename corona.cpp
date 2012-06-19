/*
 *   Copyright 2007 Matt Broadstone <mbroadst@gmail.com>
 *   Copyright 2007-2011 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 Riccardo Iaconelli <riccardo@kde.org>
 *   Copyright (c) 2009 Chani Armitage <chani@kde.org>
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

#include "corona.h"
#include "graphicsview/private/corona_p.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QGraphicsView>
#include <QGraphicsGridLayout>
#include <QMimeData>
#include <QPainter>
#include <QTimer>

#include <cmath>

#include <kaction.h>
#include <kdebug.h>
#include <klocalizedstring.h>

#include "coronabase.h"
#include "abstractdialogmanager.h"
#include "abstracttoolbox.h"
#include "containment.h"
#include "containmentactionspluginsconfig.h"
#include "pluginloader.h"
#include "private/applet_p.h"
#include "private/containment_p.h"
#include "tooltipmanager.h"
#include "view.h"

using namespace Plasma;

namespace Plasma
{

bool CoronaPrivate::s_positioningContainments = false;

Corona::Corona(QObject *parent)
    : QGraphicsScene(parent),
      d(new CoronaPrivate(this))
{
#ifndef NDEBUG
    kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "Corona ctor start";
#endif
    d->coronaBase = new CoronaBase(this);
    d->init();
    //setViewport(new QGLWidget(QGLFormat(QGL::StencilBuffer | QGL::AlphaChannel)));
}

Corona::~Corona()
{
    KConfigGroup trans(KSharedConfig::openConfig(), "PlasmaTransientsConfig");
    trans.deleteGroup();

    // FIXME: Same fix as in Plasma::View - make sure that when the focused widget is
    //        destroyed we don't try to transfer it to something that's already been
    //        deleted.
    clearFocus();
    delete d;
}

void Corona::setAppletMimeType(const QString &type)
{
    d->coronaBase->setAppletMimeType(type);
}

QString Corona::appletMimeType()
{
    return d->coronaBase->appletMimeType();
}

void Corona::setDefaultContainmentPlugin(const QString &name)
{
    d->coronaBase->setDefaultContainmentPlugin(name);
}

QString Corona::defaultContainmentPlugin() const
{
    return d->coronaBase->defaultContainmentPlugin();
}

void Corona::saveLayout(const QString &configName) const
{
    d->coronaBase->saveLayout(configName);
}

void Corona::exportLayout(KConfigGroup &config, QList<Containment*> containments)
{
    d->coronaBase->exportLayout(config, containments);
}

void Corona::requestConfigSync()
{
    d->coronaBase->requestConfigSync();
}

void Corona::requireConfigSync()
{
    d->coronaBase->requestConfigSync();
}

void Corona::initializeLayout(const QString &configName)
{
    d->coronaBase->initializeLayout();
}

bool containmentSortByPosition(const Containment *c1, const Containment *c2)
{
    return c1->id() < c2->id();
}

void Corona::layoutContainments()
{
    if (CoronaPrivate::s_positioningContainments) {
        return;
    }

    CoronaPrivate::s_positioningContainments = true;

    //TODO: we should avoid running this too often; consider compressing requests
    //      with a timer.
    QList<Containment*> c = containments();
    QMutableListIterator<Containment*> it(c);

    while (it.hasNext()) {
        Containment *containment = it.next();
        if (containment->containmentType() == Containment::PanelContainment ||
            containment->containmentType() == Containment::CustomPanelContainment ||
            offscreenWidgets().contains(containment)) {
            // weed out all containments we don't care about at all
            // e.g. Panels and ourself
            it.remove();
            continue;
        }
    }

    qSort(c.begin(), c.end(), containmentSortByPosition);

    if (c.isEmpty()) {
        CoronaPrivate::s_positioningContainments = false;
        return;
    }

    int column = 0;
    int x = 0;
    int y = 0;
    int rowHeight = 0;

    it.toFront();
    while (it.hasNext()) {
        Containment *containment = it.next();
        containment->setPos(x, y);
        //kDebug() << ++count << "setting to" << x << y;

        int height = containment->size().height();
        if (height > rowHeight) {
            rowHeight = height;
        }

        ++column;

        if (column == CONTAINMENT_COLUMNS) {
            column = 0;
            x = 0;
            y += rowHeight + INTER_CONTAINMENT_MARGIN + TOOLBOX_MARGIN;
            rowHeight = 0;
        } else {
            x += containment->size().width() + INTER_CONTAINMENT_MARGIN;
        }
        //kDebug() << "column: " << column << "; x " << x << "; y" << y << "; width was"
        //         << containment->size().width();
    }

    CoronaPrivate::s_positioningContainments = false;
}


void Corona::loadLayout(const QString &configName)
{
    d->coronaBase->loadLayout();
}

QList<Plasma::Containment *> Corona::importLayout(const KConfigGroup &conf)
{
    return d->coronaBase->importLayout(conf);
}

Containment *Corona::containmentForScreen(int screen, int desktop) const
{
    return d->coronaBase->containmentForScreen(screen, desktop);
}

Containment *Corona::containmentForScreen(int screen, int desktop,
                                          const QString &defaultPluginIfNonExistent, const QVariantList &defaultArgs)
{
    return d->coronaBase->containmentForScreen(screen, desktop, defaultPluginIfNonExistent, defaultArgs);
}

QList<Containment*> Corona::containments() const
{
    return d->coronaBase->containments();
}

void Corona::clearContainments()
{
    d->coronaBase->clearContainments();
}

KSharedConfigPtr Corona::config() const
{
    return d->coronaBase->config();
}

Containment *Corona::addContainment(const QString &name, const QVariantList &args)
{
    Plasma::Containment *containment = d->coronaBase->addContainment(name, args);
    addItem(containment);
    return containment;
}

Containment *Corona::addContainmentDelayed(const QString &name, const QVariantList &args)
{
    Plasma::Containment *containment = d->coronaBase->addContainmentDelayed(name, args);
    addItem(containment);
    return containment;
}

void Corona::addOffscreenWidget(QGraphicsWidget *widget)
{
    foreach (QGraphicsWidget *w, d->offscreenWidgets) {
        if (w == widget) {
#ifndef NDEBUG
            kDebug() << "widget is already an offscreen widget!";
#endif
            return;
        }
    }

    //search for an empty spot in the topleft quadrant of the scene. each 'slot' is QWIDGETSIZE_MAX
    //x QWIDGETSIZE_MAX, so we're guaranteed to never have to move widgets once they're placed here.
    int i = 0;
    while (d->offscreenWidgets.contains(i)) {
        i++;
    }

    d->offscreenWidgets[i] = widget;
    widget->setPos((-i - 1) * QWIDGETSIZE_MAX, -QWIDGETSIZE_MAX);

    QGraphicsWidget *pw = widget->parentWidget();
    widget->setParentItem(0);
    if (pw) {
        widget->setParent(pw);
    }

    //kDebug() << "adding offscreen widget at slot " << i;
    if (!widget->scene()) {
        addItem(widget);
    }

    connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(offscreenWidgetDestroyed(QObject*)));
}

void Corona::removeOffscreenWidget(QGraphicsWidget *widget)
{
    QMutableHashIterator<uint, QGraphicsWidget *> it(d->offscreenWidgets);

    while (it.hasNext()) {
        if (it.next().value() == widget) {
            it.remove();
            return;
        }
    }
}

QList <QGraphicsWidget *> Corona::offscreenWidgets() const
{
    return d->offscreenWidgets.values();
}

void CoronaPrivate::offscreenWidgetDestroyed(QObject *o)
{
    // at this point, it's just a QObject, not a QGraphicsWidget, but we still need
    // a pointer of the appropriate type.
    // WARNING: DO NOT USE THE WIDGET POINTER FOR ANYTHING OTHER THAN POINTER COMPARISONS
    QGraphicsWidget *widget = static_cast<QGraphicsWidget *>(o);
    q->removeOffscreenWidget(widget);
}

int Corona::numScreens() const
{
    return 1;
}

QRect Corona::screenGeometry(int id) const
{
    Q_UNUSED(id);
    QGraphicsView *v = views().value(0);
    if (v) {
        QRect r = sceneRect().toRect();
        r.moveTo(v->mapToGlobal(QPoint(0, 0)));
        return r;
    }

    return sceneRect().toRect();
}

QRegion Corona::availableScreenRegion(int id) const
{
    return QRegion(screenGeometry(id));
}

QPoint Corona::popupPosition(const QGraphicsItem *item, const QSize &s, Qt::AlignmentFlag alignment)
{
    const QGraphicsItem *actualItem = item;
    const QGraphicsView *v = viewFor(item);

    if (!v) {
        return QPoint(0, 0);
    }

    //its own view could be hidden, for instance if item is in an hidden Dialog
    //try to position it using the parent applet as the item
    if (!v->isVisible()) {
        actualItem = item->parentItem();
        if (!actualItem) {
            const QGraphicsWidget *widget = qgraphicsitem_cast<const QGraphicsWidget*>(item);
            if (widget) {
                actualItem = qobject_cast<QGraphicsItem*>(widget->parent());
            }
        }

#ifndef NDEBUG
        kDebug() << actualItem;
#endif

        if (actualItem) {
            v = viewFor(actualItem);
            if (!v) {
                return QPoint(0, 0);
            }
        }
    }

    if (!actualItem) {
        actualItem = item;
    }

    QPoint pos;
    QTransform sceneTransform = actualItem->sceneTransform();

    //swap direction if necessary
    if (QApplication::isRightToLeft() && alignment != Qt::AlignCenter) {
        if (alignment == Qt::AlignRight) {
            alignment = Qt::AlignLeft;
        } else {
            alignment = Qt::AlignRight;
        }
    }

    //if the applet is rotated the popup position has to be un-transformed
    if (sceneTransform.isRotating()) {
        qreal angle = acos(sceneTransform.m11());
        QTransform newTransform;
        QPointF center = actualItem->sceneBoundingRect().center();

        newTransform.translate(center.x(), center.y());
        newTransform.rotateRadians(-angle);
        newTransform.translate(-center.x(), -center.y());
        pos = v->mapFromScene(newTransform.inverted().map(actualItem->scenePos()));
    } else {
        pos = v->mapFromScene(actualItem->scenePos());
    }

    pos = v->mapToGlobal(pos);
    //kDebug() << "==> position is" << actualItem->scenePos() << v->mapFromScene(actualItem->scenePos()) << pos;
    const Plasma::View *pv = qobject_cast<const Plasma::View *>(v);

    Plasma::Location loc = Floating;
    if (pv && pv->containment()) {
        loc = pv->containment()->location();
    }

    switch (loc) {
    case BottomEdge:
    case TopEdge: {
        if (alignment == Qt::AlignCenter) {
            pos.setX(pos.x() + actualItem->boundingRect().width()/2 - s.width()/2);
        } else if (alignment == Qt::AlignRight) {
            pos.setX(pos.x() + actualItem->boundingRect().width() - s.width());
        }

        if (pos.x() + s.width() > v->geometry().x() + v->geometry().width()) {
            pos.setX((v->geometry().x() + v->geometry().width()) - s.width());
        } else {
            pos.setX(qMax(pos.x(), v->geometry().left()));
        }
        break;
    }
    case LeftEdge:
    case RightEdge: {
        if (alignment == Qt::AlignCenter) {
            pos.setY(pos.y() + actualItem->boundingRect().height()/2 - s.height()/2);
        } else if (alignment == Qt::AlignRight) {
            pos.setY(pos.y() + actualItem->boundingRect().height() - s.height());
        }

        if (pos.y() + s.height() > v->geometry().y() + v->geometry().height()) {
            pos.setY((v->geometry().y() + v->geometry().height()) - s.height());
        } else {
            pos.setY(qMax(pos.y(), v->geometry().top()));
        }
        break;
    }
    default:
        if (alignment == Qt::AlignCenter) {
            pos.setX(pos.x() + actualItem->boundingRect().width()/2 - s.width()/2);
        } else if (alignment == Qt::AlignRight) {
            pos.setX(pos.x() + actualItem->boundingRect().width() - s.width());
        }
        break;
    }


    //are we out of screen?
    int screen = ((pv && pv->containment()) ? pv->containment()->screen() : -1);
    if (screen == -1) {
        if (pv) {
            screen = pv->screen();
        } else {
            // fall back to asking the actual system what screen the view is on
            // in the case we are dealing with a non-PlasmaView QGraphicsView
            screen = QApplication::desktop()->screenNumber(v);
        }
    }

    QRect screenRect = screenGeometry(screen);

    switch (loc) {
    case BottomEdge:
        pos.setY(v->geometry().y() - s.height());
        break;
    case TopEdge:
        pos.setY(v->geometry().y() + v->geometry().height());
        break;
    case LeftEdge:
        pos.setX(v->geometry().x() + v->geometry().width());
        break;
    case RightEdge:
        pos.setX(v->geometry().x() - s.width());
        break;
    default:
        if (pos.y() - s.height() > screenRect.top()) {
             pos.ry() = pos.y() - s.height();
        } else {
             pos.ry() = pos.y() + (int)actualItem->boundingRect().size().height() + 1;
        }
    }

    //kDebug() << "==> rect for" << screen << "is" << screenRect;

    if (loc != LeftEdge && pos.x() + s.width() > screenRect.x() + screenRect.width()) {
        pos.rx() -= ((pos.x() + s.width()) - (screenRect.x() + screenRect.width()));
    }

    if (loc != TopEdge && pos.y() + s.height() > screenRect.y() + screenRect.height()) {
        pos.ry() -= ((pos.y() + s.height()) - (screenRect.y() + screenRect.height()));
    }

    pos.rx() = qMax(0, pos.x());
    pos.ry() = qMax(0, pos.y());
    return pos;
}

void Corona::loadDefaultLayout()
{
}

void Corona::setPreferredToolBoxPlugin(const Containment::Type type, const QString &plugin)
{
    d->coronaBase->setPreferredToolBoxPlugin(type, plugin);
}

QString Corona::preferredToolBoxPlugin(const Containment::Type type) const
{
    return d->coronaBase->preferredToolBoxPlugin(type);
}

ImmutabilityType Corona::immutability() const
{
    return d->coronaBase->immutability();
}

void Corona::setImmutability(const ImmutabilityType immutable)
{
    d->coronaBase->setImmutability(immutable);
}

QList<Plasma::Location> Corona::freeEdges(int screen) const
{
    return d->coronaBase->freeEdges(screen);
}

QAction *Corona::action(QString name) const
{
    return d->coronaBase->action(name);
}

void Corona::addAction(QString name, QAction *action)
{
    d->coronaBase->addAction(name, action);
}

KAction* Corona::addAction(QString name)
{
    return d->coronaBase->addAction(name);
}

QList<QAction*> Corona::actions() const
{
    return d->coronaBase->actions();
}

void Corona::enableAction(const QString &name, bool enable)
{
    d->coronaBase->enableAction(name, enable);
}

void Corona::updateShortcuts()
{
    d->coronaBase->updateShortcuts();
}

void Corona::addShortcuts(KActionCollection *newShortcuts)
{
    d->coronaBase->addShortcuts(newShortcuts);
}

void Corona::setContainmentActionsDefaults(Containment::Type containmentType, const ContainmentActionsPluginsConfig &config)
{
    d->coronaBase->setContainmentActionsDefaults(containmentType, config);
}

ContainmentActionsPluginsConfig Corona::containmentActionsDefaults(Containment::Type containmentType)
{
    return d->coronaBase->containmentActionsDefaults(containmentType);
}

void Corona::setDialogManager(AbstractDialogManager *dialogManager)
{
    d->coronaBase->setDialogManager(dialogManager);
}

AbstractDialogManager *Corona::dialogManager()
{
    return d->coronaBase->dialogManager();
}

CoronaPrivate::CoronaPrivate(Corona *corona)
    : q(corona)
{
}

CoronaPrivate::~CoronaPrivate()
{
}

void CoronaPrivate::init()
{
    q->setStickyFocus(true);
}


} // namespace Plasma



#include "moc_corona.cpp"
