/*
 *   Copyright 2013 Marco Martin <mart@kde.org>
 *   Copyright 2015 Eike Hein <hein@kde.org>
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

#include "configcategory_p.h"

namespace PlasmaQuick
{

///////////////////////ConfigCategory

ConfigCategory::ConfigCategory(QObject *parent)
    : QObject(parent)
    , m_visible(true)
{
}

ConfigCategory::~ConfigCategory()
{}

QString ConfigCategory::name() const
{
    return m_name;
}

void ConfigCategory::setName(const QString &name)
{
    if (m_name == name) {
        return;
    }

    m_name = name;
    emit nameChanged();
}

QString ConfigCategory::icon() const
{
    return m_icon;
}

void ConfigCategory::setIcon(const QString &icon)
{
    if (m_icon == icon) {
        return;
    }

    m_icon = icon;
    emit iconChanged();
}

QString ConfigCategory::source() const
{
    return m_source;
}

void ConfigCategory::setSource(const QString &source)
{
    if (m_source == source) {
        return;
    }

    m_source = source;
    emit sourceChanged();
}

QString ConfigCategory::pluginName() const
{
    return m_pluginName;
}

void ConfigCategory::setPluginName(const QString &name)
{
    if (m_pluginName == name) {
        return;
    }

    m_pluginName = name;
    emit pluginNameChanged();
}

bool ConfigCategory::visible() const
{
    return m_visible;
}

void ConfigCategory::setVisible(bool visible)
{
    if (m_visible == visible) {
        return;
    }

    m_visible = visible;
    emit visibleChanged();
}

}

#include "private/moc_configcategory_p.cpp"
