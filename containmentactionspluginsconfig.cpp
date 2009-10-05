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

#include "containmentactionspluginsconfig.h"
#include "private/containmentactionspluginsconfig_p.h"

#include <QHash>
#include <QEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QString>

#include <kdebug.h>

#include "containmentactions.h"

using namespace Plasma;

namespace Plasma
{

ContainmentActionsPluginsConfig::ContainmentActionsPluginsConfig()
    : d(new ContainmentActionsPluginsConfigPrivate(this))
{
}

ContainmentActionsPluginsConfig::ContainmentActionsPluginsConfig(const ContainmentActionsPluginsConfig &other)
    : d(new ContainmentActionsPluginsConfigPrivate(this))
{
    d->plugins = other.d->plugins;
}

ContainmentActionsPluginsConfig& ContainmentActionsPluginsConfig::operator=(const ContainmentActionsPluginsConfig &other)
{
    d->plugins = other.d->plugins;
    return *this;
}

ContainmentActionsPluginsConfig::~ContainmentActionsPluginsConfig()
{
    delete d;
}

void ContainmentActionsPluginsConfig::clear()
{
    d->plugins.clear();
}

void ContainmentActionsPluginsConfig::remove(QEvent *trigger)
{
    QString s = ContainmentActions::eventToString(trigger);
    d->plugins.remove(s);
}

void ContainmentActionsPluginsConfig::addPlugin(QEvent *trigger, const QString &name)
{
    QString s = ContainmentActions::eventToString(trigger);
    d->plugins.insert(s, name);
}

void ContainmentActionsPluginsConfig::addPlugin(Qt::KeyboardModifiers modifiers, Qt::MouseButton button, const QString &name)
{
    QGraphicsSceneMouseEvent event(QEvent::GraphicsSceneMousePress);
    event.setButton(button);
    event.setModifiers(modifiers);
    QString s = ContainmentActions::eventToString(&event);
    d->plugins.insert(s, name);
}

void ContainmentActionsPluginsConfig::addPlugin(Qt::KeyboardModifiers modifiers, Qt::Orientation wheelDirection, const QString &name)
{
    QGraphicsSceneWheelEvent event(QEvent::GraphicsSceneWheel);
    event.setOrientation(wheelDirection);
    event.setModifiers(modifiers);
    QString s = ContainmentActions::eventToString(&event);
    d->plugins.insert(s, name);
}

} // namespace Plasma

