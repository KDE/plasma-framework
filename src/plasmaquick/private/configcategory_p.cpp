/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
{
}

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
    Q_EMIT nameChanged();
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
    Q_EMIT iconChanged();
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
    Q_EMIT sourceChanged();
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
    Q_EMIT pluginNameChanged();
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
    Q_EMIT visibleChanged();
}

}

#include "private/moc_configcategory_p.cpp"
