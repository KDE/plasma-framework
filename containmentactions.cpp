/*
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

#include "containmentactions.h"
#include "containment.h"

#include "private/dataengineconsumer_p.h"
#include "private/packages_p.h"
#include "private/containmentactions_p.h"
#include "private/containment_p.h"

#include <QMetaEnum>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>

#include <kdebug.h>
#include <kglobal.h>
#include <kservicetypetrader.h>
#include <kstandarddirs.h>

#include <version.h>

namespace Plasma
{

PackageStructure::Ptr ContainmentActionsPrivate::s_packageStructure(0);

ContainmentActions::ContainmentActions(QObject * parentObject)
    : d(new ContainmentActionsPrivate(KService::serviceByStorageId(QString()), this))
{
    setParent(parentObject);
}

ContainmentActions::ContainmentActions(QObject *parentObject, const QVariantList &args)
    : d(new ContainmentActionsPrivate(KService::serviceByStorageId(args.count() > 0 ?
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

ContainmentActions::~ContainmentActions()
{
    delete d;
}

KPluginInfo::List ContainmentActions::listContainmentActionsInfo()
{
    QString constraint;

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/ContainmentActions", constraint);
    return KPluginInfo::fromServices(offers);
}

ContainmentActions *ContainmentActions::load(Containment *parent, const QString &containmentActionsName, const QVariantList &args)
{
    if (!parent) {
        return 0;
    }

    if (containmentActionsName.isEmpty()) {
        return 0;
    }

    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(containmentActionsName);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/ContainmentActions", constraint);

    if (offers.isEmpty()) {
        kDebug() << "offers is empty for " << containmentActionsName;
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
    ContainmentActions *containmentActions = offer->createInstance<Plasma::ContainmentActions>(parent, allArgs, &error);

    if (!containmentActions) {
        kDebug() << "Couldn't load containmentActions \"" << containmentActionsName << "\"! reason given: " << error;
    }

    return containmentActions;
}

ContainmentActions *ContainmentActions::load(Containment *parent, const KPluginInfo &info, const QVariantList &args)
{
    if (!info.isValid()) {
        return 0;
    }
    return load(parent, info.pluginName(), args);
}

PackageStructure::Ptr ContainmentActions::packageStructure()
{
    if (!ContainmentActionsPrivate::s_packageStructure) {
        ContainmentActionsPrivate::s_packageStructure = new ContainmentActionsPackage();
    }

    return ContainmentActionsPrivate::s_packageStructure;
}

Containment *ContainmentActions::containment()
{
    return qobject_cast<Containment*>(parent());
}

QString ContainmentActions::name() const
{
    if (!d->containmentActionsDescription.isValid()) {
        return i18n("Unknown ContainmentActions");
    }

    return d->containmentActionsDescription.name();
}

QString ContainmentActions::icon() const
{
    if (!d->containmentActionsDescription.isValid()) {
        return QString();
    }

    return d->containmentActionsDescription.icon();
}

QString ContainmentActions::pluginName() const
{
    if (!d->containmentActionsDescription.isValid()) {
        return QString();
    }

    return d->containmentActionsDescription.pluginName();
}

bool ContainmentActions::isInitialized() const
{
    return d->initialized;
}

void ContainmentActions::restore(const KConfigGroup &config)
{
    init(config);
    d->initialized = true;
}

void ContainmentActions::init(const KConfigGroup &config)
{
    Q_UNUSED(config);
}

void ContainmentActions::save(KConfigGroup &config)
{
    Q_UNUSED(config);
}

QWidget *ContainmentActions::createConfigurationInterface(QWidget *parent)
{
    Q_UNUSED(parent);
    return 0;
}

void ContainmentActions::configurationAccepted()
{
    //do nothing by default
}

void ContainmentActions::contextEvent(QEvent *event)
{
    Q_UNUSED(event)
}

QList<QAction*> ContainmentActions::contextualActions()
{
    //empty list
    return QList<QAction*>();
}

DataEngine *ContainmentActions::dataEngine(const QString &name) const
{
    return d->dataEngine(name);
}

bool ContainmentActions::configurationRequired() const
{
    return d->needsConfig;
}

void ContainmentActions::setConfigurationRequired(bool needsConfig)
{
    //TODO: reason?
    d->needsConfig = needsConfig;
}

QString ContainmentActions::eventToString(QEvent *event)
{
    QString trigger;
    Qt::KeyboardModifiers modifiers;

    switch (event->type()) {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        {
            QMouseEvent *e = static_cast<QMouseEvent*>(event);
            int m = QObject::staticQtMetaObject.indexOfEnumerator("MouseButtons");
            QMetaEnum mouse = QObject::staticQtMetaObject.enumerator(m);
            trigger += mouse.valueToKey(e->button());
            modifiers = e->modifiers();
            break;
        }
        case QEvent::GraphicsSceneMousePress:
        case QEvent::GraphicsSceneMouseRelease:
        case QEvent::GraphicsSceneMouseDoubleClick:
        {
            QGraphicsSceneMouseEvent *e = static_cast<QGraphicsSceneMouseEvent*>(event);
            int m = QObject::staticQtMetaObject.indexOfEnumerator("MouseButtons");
            QMetaEnum mouse = QObject::staticQtMetaObject.enumerator(m);
            trigger += mouse.valueToKey(e->button());
            modifiers = e->modifiers();
            break;
        }
        case QEvent::Wheel:
        {
            QWheelEvent *e = static_cast<QWheelEvent*>(event);
            int o = QObject::staticQtMetaObject.indexOfEnumerator("Orientations");
            QMetaEnum orient = QObject::staticQtMetaObject.enumerator(o);
            trigger = "wheel:";
            trigger += orient.valueToKey(e->orientation());
            modifiers = e->modifiers();
            break;
        }
        case QEvent::GraphicsSceneWheel:
        {
            QGraphicsSceneWheelEvent *e = static_cast<QGraphicsSceneWheelEvent*>(event);
            int o = QObject::staticQtMetaObject.indexOfEnumerator("Orientations");
            QMetaEnum orient = QObject::staticQtMetaObject.enumerator(o);
            trigger = "wheel:";
            trigger += orient.valueToKey(e->orientation());
            modifiers = e->modifiers();
            break;
        }
        case QEvent::GraphicsSceneContextMenu:
        case QEvent::ContextMenu:
        {
            int m = QObject::staticQtMetaObject.indexOfEnumerator("MouseButtons");
            QMetaEnum mouse = QObject::staticQtMetaObject.enumerator(m);
            trigger = mouse.valueToKey(Qt::RightButton);
            modifiers = Qt::NoModifier;
            break;
        }
        default:
            return QString();
    }

    int k = QObject::staticQtMetaObject.indexOfEnumerator("KeyboardModifiers");
    QMetaEnum kbd = QObject::staticQtMetaObject.enumerator(k);
    trigger += ';';
    trigger += kbd.valueToKeys(modifiers);

    return trigger;
}

void ContainmentActions::paste(QPointF scenePos, QPoint screenPos)
{
    Containment *c = containment();
    if (c) {
        c->d->dropData(scenePos, screenPos);
    }
}

QPoint screenPosFromEvent(QEvent *event)
{
    switch (event->type()) {
        case QEvent::GraphicsSceneMousePress:
        case QEvent::GraphicsSceneMouseRelease:
        case QEvent::GraphicsSceneMouseDoubleClick:
            return static_cast<QGraphicsSceneMouseEvent*>(event)->screenPos();
            break;
        case QEvent::GraphicsSceneWheel:
            return static_cast<QGraphicsSceneWheelEvent*>(event)->screenPos();
            break;
        case QEvent::GraphicsSceneContextMenu:
            return static_cast<QGraphicsSceneContextMenuEvent*>(event)->screenPos();
            break;
        default:
            break;
    }

    return QPoint();
}

QPointF scenePosFromEvent(QEvent *event)
{
    switch (event->type()) {
        case QEvent::GraphicsSceneMousePress:
        case QEvent::GraphicsSceneMouseRelease:
        case QEvent::GraphicsSceneMouseDoubleClick:
            return static_cast<QGraphicsSceneMouseEvent*>(event)->scenePos();
            break;
        case QEvent::GraphicsSceneWheel:
            return static_cast<QGraphicsSceneWheelEvent*>(event)->scenePos();
            break;
        case QEvent::GraphicsSceneContextMenu:
            return static_cast<QGraphicsSceneContextMenuEvent*>(event)->scenePos();
            break;
        default:
            break;
    }

    return QPoint();
}

bool isNonSceneEvent(QEvent *event)
{
    return dynamic_cast<QGraphicsSceneEvent *>(event) == 0;
}

QPoint ContainmentActions::popupPosition(const QSize &s, QEvent *event)
{
    if (isNonSceneEvent(event)) {
        return screenPosFromEvent(event);
    }

    Containment *c = containment();
    if (!c) {
        return screenPosFromEvent(event);
    }

    Applet *applet = c->d->appletAt(scenePosFromEvent(event));
    QPoint screenPos = screenPosFromEvent(event);
    QPoint pos = screenPos;
    if (applet && containment()->d->isPanelContainment()) {
        pos = applet->popupPosition(s);
        if (event->type() != QEvent::GraphicsSceneContextMenu ||
            static_cast<QGraphicsSceneContextMenuEvent *>(event)->reason() == QGraphicsSceneContextMenuEvent::Mouse) {
            // if the menu pops up way away from the mouse press, then move it
            // to the mouse press
            if (c->formFactor() == Vertical) {
                if (pos.y() + s.height() < screenPos.y()) {
                    pos.setY(screenPos.y());
                }
            } else if (c->formFactor() == Horizontal) {
                if (pos.x() + s.width() < screenPos.x()) {
                    pos.setX(screenPos.x());
                }
            }
        }
    }

    return pos;
}

bool ContainmentActions::event(QEvent *e)
{
    if (e->type() == QEvent::ParentChange) {
        if (!containment()) {
            //some fool took away our containment. run away, run away!
            deleteLater();
        }
    }
    return false;
}

} // Plasma namespace

#include "containmentactions.moc"
