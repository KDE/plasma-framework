/*
    SPDX-FileCopyrightText: 2009 Chani Armitage <chani@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "containmentactions.h"
#include "containment.h"

#include "private/containment_p.h"
#include "private/containmentactions_p.h"

#include <QContextMenuEvent>
#include <QDebug>
#include <QMetaEnum>
#include <QMouseEvent>
#include <QWheelEvent>

#include <KLocalizedString>

#include "version.h"

namespace Plasma
{
ContainmentActions::ContainmentActions(QObject *parentObject)
    : d(new ContainmentActionsPrivate({}, this))
{
    setParent(parentObject);
}

ContainmentActions::ContainmentActions(QObject *parentObject, const QVariantList &args)
    : d(new ContainmentActionsPrivate(args.value(0), this))
{
    setParent(parentObject);

    // now remove first item since those are managed by Wallpaper and subclasses shouldn't
    // need to worry about them. yes, it violates the constness of this var, but it lets us add
    // or remove items later while applets can just pretend that their args always start at 0
    QVariantList &mutableArgs = const_cast<QVariantList &>(args);
    if (!mutableArgs.isEmpty()) {
        mutableArgs.removeFirst();
    }
}

ContainmentActions::~ContainmentActions()
{
    delete d;
}

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 67)
KPluginInfo ContainmentActions::pluginInfo() const
{
    return KPluginInfo(d->containmentActionsDescription);
}
#endif

KPluginMetaData ContainmentActions::metadata() const
{
    return d->containmentActionsDescription;
}

Containment *ContainmentActions::containment()
{
    if (d->containment) {
        return d->containment;
    }
    return qobject_cast<Containment *>(parent());
}

void ContainmentActions::restore(const KConfigGroup &config)
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
    return nullptr;
}

void ContainmentActions::configurationAccepted()
{
    // do nothing by default
}

void ContainmentActions::performNextAction()
{
    // do nothing by default, implement in subclasses
}

void ContainmentActions::performPreviousAction()
{
    // do nothing by default, implement in subclasses
}

QList<QAction *> ContainmentActions::contextualActions()
{
    return QList<QAction *>();
}

QString ContainmentActions::eventToString(QEvent *event)
{
    QString trigger;
    Qt::KeyboardModifiers modifiers;

    switch (event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick: {
        QMouseEvent *e = static_cast<QMouseEvent *>(event);
        int m = QObject::staticQtMetaObject.indexOfEnumerator("MouseButtons");
        QMetaEnum mouse = QObject::staticQtMetaObject.enumerator(m);
        trigger += QString::fromLatin1(mouse.valueToKey(e->button()));
        modifiers = e->modifiers();
        break;
    }
    case QEvent::Wheel: {
        QWheelEvent *e = static_cast<QWheelEvent *>(event);
        trigger = QStringLiteral("wheel:");
        // ContainmentInterface::wheelEvent uses angleDelta.y()
        // To support both, should we just remove this orientation string?
        trigger += QStringLiteral("Vertical");
        modifiers = e->modifiers();
        break;
    }
    case QEvent::ContextMenu: {
        int m = QObject::staticQtMetaObject.indexOfEnumerator("MouseButtons");
        QMetaEnum mouse = QObject::staticQtMetaObject.enumerator(m);
        trigger = QString::fromLatin1(mouse.valueToKey(Qt::RightButton));
        modifiers = Qt::NoModifier;
        break;
    }
    default:
        return QString();
    }

    int k = QObject::staticQtMetaObject.indexOfEnumerator("KeyboardModifiers");
    QMetaEnum kbd = QObject::staticQtMetaObject.enumerator(k);
    trigger += QLatin1Char(';') + QString::fromLatin1(kbd.valueToKeys(modifiers));

    return trigger;
}

void ContainmentActions::setContainment(Containment *newContainment)
{
    d->containment = newContainment;
}

} // Plasma namespace

#include "moc_containmentactions.cpp"
