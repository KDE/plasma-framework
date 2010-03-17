/*
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
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

#include "applet.h"

#include <QAction>
#include <QGraphicsLinearLayout>

#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/Corona>

Applet::Applet(QObject *parent)
    : QObject(parent),
      m_configDirty(false)
{
}

Applet::~Applet()
{
    if (m_configDirty) {
        reloadConfig();
    }
}

void Applet::setCurrentConfigGroup(const QStringList &groupNames)
{
    Plasma::Applet *app = applet();
    if (!app) {
        m_configGroup = KConfigGroup();
        m_configGroupPath.clear();
        return;
    }

    m_configGroup = app->config();
    m_configGroupPath = groupNames;

    foreach (const QString &groupName, groupNames) {
        m_configGroup = KConfigGroup(&m_configGroup, groupName);
    }
}

QStringList Applet::currentConfigGroup() const
{
    return m_configGroupPath;
}

QStringList Applet::configKeys() const
{
    if (m_configGroup.isValid()) {
        return m_configGroup.keyList();
    }

    return QStringList();
}

QStringList Applet::configGroups() const
{
    if (m_configGroup.isValid()) {
        return m_configGroup.groupList();
    }

    return QStringList();
}

QVariant Applet::readConfig(const QString &key, const QVariant &def) const
{
    if (m_configGroup.isValid()) {
        return m_configGroup.readEntry(key, def);
    } else {
        return QVariant();
    }
}

void Applet::writeConfig(const QString &key, const QVariant &value)
{
    if (m_configGroup.isValid()) {
        m_configGroup.writeEntry(key, value);
        m_configDirty = true;
    }
}

void Applet::reloadConfig()
{
    Plasma::Applet *app = applet();
    if (app) {
        KConfigGroup cg = app->config();

        if (!app->isContainment()) {
            app->restore(cg);
        }

        app->configChanged();

        if (app->containment() && app->containment()->corona()) {
            app->containment()->corona()->requestConfigSync();
        }

        m_configDirty = false;
    }
}

Plasma::Applet *Applet::applet() const
{
    return 0;
}

#include "applet.moc"

