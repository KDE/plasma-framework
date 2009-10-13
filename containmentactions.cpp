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

    //strict typing sucks sometimes.
    switch (event->type()) {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        {
            QMouseEvent *e = dynamic_cast<QMouseEvent*>(event);
            int m = QObject::staticQtMetaObject.indexOfEnumerator("MouseButtons");
            QMetaEnum mouse = QObject::staticQtMetaObject.enumerator(m);
            trigger += mouse.valueToKey(e->button());
            modifiers = e->modifiers();
            break;
        }
        case QEvent::GraphicsSceneMousePress:
        case QEvent::GraphicsSceneMouseRelease:
        {
            QGraphicsSceneMouseEvent *e = dynamic_cast<QGraphicsSceneMouseEvent*>(event);
            int m = QObject::staticQtMetaObject.indexOfEnumerator("MouseButtons");
            QMetaEnum mouse = QObject::staticQtMetaObject.enumerator(m);
            trigger += mouse.valueToKey(e->button());
            modifiers = e->modifiers();
            break;
        }
        case QEvent::Wheel:
        {
            QWheelEvent *e = dynamic_cast<QWheelEvent*>(event);
            int o = QObject::staticQtMetaObject.indexOfEnumerator("Orientations");
            QMetaEnum orient = QObject::staticQtMetaObject.enumerator(o);
            trigger = "wheel:";
            trigger += orient.valueToKey(e->orientation());
            modifiers = e->modifiers();
            break;
        }
        case QEvent::GraphicsSceneWheel:
        {
            QGraphicsSceneWheelEvent *e = dynamic_cast<QGraphicsSceneWheelEvent*>(event);
            int o = QObject::staticQtMetaObject.indexOfEnumerator("Orientations");
            QMetaEnum orient = QObject::staticQtMetaObject.enumerator(o);
            trigger = "wheel:";
            trigger += orient.valueToKey(e->orientation());
            modifiers = e->modifiers();
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
