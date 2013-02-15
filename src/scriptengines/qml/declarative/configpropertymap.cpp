/*
 *   Copyright 2013 Marco Martin <notmart@gmail.com>
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

#include "configpropertymap.h"

#include <QDebug>

#include <KConfigSkeletonItem>


ConfigPropertyMap::ConfigPropertyMap(KConfigSkeleton *config, QObject *parent)
    : QQmlPropertyMap(parent),
      m_config(config)
{
    connect(config, &KConfigSkeleton::configChanged,
            this, &ConfigPropertyMap::loadConfig);
    connect(this, &ConfigPropertyMap::valueChanged,
            this, &ConfigPropertyMap::writeConfigValue);

    loadConfig();
}

ConfigPropertyMap::~ConfigPropertyMap()
{
    writeConfig();
}

void ConfigPropertyMap::loadConfig()
{
    if (!m_config) {
        return;
    }

    foreach (KConfigSkeletonItem *item, m_config.data()->items()) {
        insert(item->key(), item->property());
    }
}

void ConfigPropertyMap::writeConfig()
{
    if (!m_config) {
        return;
    }

    foreach (KConfigSkeletonItem *item, m_config.data()->items()) {
        item->setProperty(value(item->key()));
    }

    m_config.data()->blockSignals(true);
    m_config.data()->writeConfig();
    m_config.data()->blockSignals(false);
}

void ConfigPropertyMap::writeConfigValue(const QString &key, const QVariant &value)
{
    KConfigSkeletonItem *item = m_config.data()->findItem(key);
    if (item) {
        item->setProperty(value);
        m_config.data()->blockSignals(true);
        m_config.data()->writeConfig();
        m_config.data()->blockSignals(false);
    }
}

#include "moc_configpropertymap.cpp"



