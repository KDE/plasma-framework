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

#include "contextaction.h"
#include "containment.h"

#include "private/dataengineconsumer_p.h"
#include "private/packages_p.h"
#include "private/contextaction_p.h"
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

PackageStructure::Ptr ContextActionPrivate::s_packageStructure(0);

ContextAction::ContextAction(QObject * parentObject)
    : d(new ContextActionPrivate(KService::serviceByStorageId(QString()), this))
{
    setParent(parentObject);
}

ContextAction::ContextAction(QObject *parentObject, const QVariantList &args)
    : d(new ContextActionPrivate(KService::serviceByStorageId(args.count() > 0 ?
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

ContextAction::~ContextAction()
{
    delete d;
}

KPluginInfo::List ContextAction::listContextActionInfo()
{
    QString constraint;

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/ContextAction", constraint);
    return KPluginInfo::fromServices(offers);
}

ContextAction *ContextAction::load(const QString &contextActionName, const QVariantList &args)
{
    if (contextActionName.isEmpty()) {
        return 0;
    }

    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(contextActionName);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/ContextAction", constraint);

    if (offers.isEmpty()) {
        kDebug() << "offers is empty for " << contextActionName;
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
    ContextAction *contextAction = offer->createInstance<Plasma::ContextAction>(0, allArgs, &error);

    if (!contextAction) {
        kDebug() << "Couldn't load contextAction \"" << contextActionName << "\"! reason given: " << error;
    }

    return contextAction;
}

ContextAction *ContextAction::load(const KPluginInfo &info, const QVariantList &args)
{
    if (!info.isValid()) {
        return 0;
    }
    return load(info.pluginName(), args);
}

PackageStructure::Ptr ContextAction::packageStructure()
{
    if (!ContextActionPrivate::s_packageStructure) {
        ContextActionPrivate::s_packageStructure = new ContextActionPackage();
    }

    return ContextActionPrivate::s_packageStructure;
}

void ContextAction::setContainment(Containment *c)
{
    d->containment=c;
}

Containment *ContextAction::containment()
{
    return d->containment;
}

QString ContextAction::name() const
{
    if (!d->contextActionDescription.isValid()) {
        return i18n("Unknown ContextAction");
    }

    return d->contextActionDescription.name();
}

QString ContextAction::icon() const
{
    if (!d->contextActionDescription.isValid()) {
        return QString();
    }

    return d->contextActionDescription.icon();
}

QString ContextAction::pluginName() const
{
    if (!d->contextActionDescription.isValid()) {
        return QString();
    }

    return d->contextActionDescription.pluginName();
}

bool ContextAction::isInitialized() const
{
    return d->initialized;
}

void ContextAction::restore(const KConfigGroup &config)
{
    init(config);
    d->initialized = true;
}

void ContextAction::init(const KConfigGroup &config)
{
    Q_UNUSED(config);
}

void ContextAction::save(KConfigGroup &config)
{
    Q_UNUSED(config);
}

QWidget *ContextAction::createConfigurationInterface(QWidget *parent)
{
    Q_UNUSED(parent);
    return 0;
}

void ContextAction::configurationAccepted()
{
    //do nothing by default
}

void ContextAction::contextEvent(QEvent *event)
{
    Q_UNUSED(event)
}

QList<QAction*> ContextAction::contextualActions()
{
    //empty list
    return QList<QAction*>();
}

DataEngine *ContextAction::dataEngine(const QString &name) const
{
    return d->dataEngine(name);
}

bool ContextAction::configurationRequired() const
{
    return d->needsConfig;
}

void ContextAction::setConfigurationRequired(bool needsConfig)
{
    //TODO: reason?
    d->needsConfig = needsConfig;
}

bool ContextAction::hasConfigurationInterface() const
{
    return d->hasConfig;
}

void ContextAction::setHasConfigurationInterface(bool hasConfig)
{
    d->hasConfig = hasConfig;
}

QString ContextAction::eventToString(QEvent *event)
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
    trigger += ";";
    trigger += kbd.valueToKeys(modifiers);

    return trigger;
}

void ContextAction::paste(QPointF scenePos, QPoint screenPos)
{
    Containment *c = containment();
    if (c) {
        c->d->dropData(scenePos, screenPos);
    }
}

} // Plasma namespace

#include "contextaction.moc"
