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

#ifndef PLASMA_CONTAINMENTACTIONSPLUGINSCONFIG_H
#define PLASMA_CONTAINMENTACTIONSPLUGINSCONFIG_H

#include <plasma/plasma_export.h>

#include <Qt>

class QString;
class QEvent;

namespace Plasma
{

class ContainmentActionsPluginsConfigPrivate;

/**
 * @class ContainmentActionsPluginsConfig plasma/containmentactionspluginsconfig.h <Plasma/ContainmentActionsPluginsConfig>
 *
 * @short A class that holds a map of triggers to plugin names
 * @since 4.4
 */
class PLASMA_EXPORT ContainmentActionsPluginsConfig
{

public:
    ContainmentActionsPluginsConfig();
    ContainmentActionsPluginsConfig(const ContainmentActionsPluginsConfig &other);
    ~ContainmentActionsPluginsConfig();
    ContainmentActionsPluginsConfig& operator=(const ContainmentActionsPluginsConfig &other);

    /**
     * clears everything
     */
    void clear();

    /**
     * removes @p trigger
     * @see addPlugin for an explanation of the @p trigger
     */
    void remove(QEvent *trigger);

    /**
     * Sets @p trigger to plugin @p name
     * if you're passing the trigger as an event, the following events are currently understood:
     * -mouse press and release events: button and modifiers
     * -mouse wheel events: direction and modifiers
     * both traditional and graphicsscene events are supported.
     */
    void addPlugin(QEvent *trigger, const QString &name);

    /**
     * Sets trigger described by @p modifiers and @p button to plugin @p name
     */
    void addPlugin(Qt::KeyboardModifiers modifiers, Qt::MouseButton button, const QString &name);

    /**
     * Sets trigger described by @p modifiers and @p wheelDirection to plugin @p name
     */
    void addPlugin(Qt::KeyboardModifiers modifiers, Qt::Orientation wheelDirection, const QString &name);

private:
    ContainmentActionsPluginsConfigPrivate *const d;

    friend class ContainmentActionsPluginsConfigPrivate;
    friend class Containment;
};

} // namespace Plasma

#endif

