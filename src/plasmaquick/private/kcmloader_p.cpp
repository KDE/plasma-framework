/*
 *   Copyright 2013 Marco Martin <mart@kde.org>
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

#include "kcmloader_p.h"

#include <QQuickWindow>
#include <QWindow>

#include <KCModuleProxy>


KCMLoader::KCMLoader(QQuickItem *parent)
    : QQuickItem(parent),
      m_module(0)
{
}

KCMLoader::~KCMLoader()
{}

QString KCMLoader::pluginName() const
{
    return m_pluginName;
}

void KCMLoader::setPluginName(const QString &name)
{
    if (m_pluginName == name) {
        return;
    }

    delete m_module;
    m_module = new KCModuleProxy(name);
    m_module->load();
    m_module->show();
    m_module->windowHandle()->setParent(window());
    m_module->windowHandle()->setX(mapToScene(QPoint()).x());
    m_module->windowHandle()->setY(mapToScene(QPoint()).y());
    m_module->windowHandle()->setWidth(width());
    m_module->windowHandle()->setHeight(height());
    m_pluginName = name;
    emit pluginNameChanged();
}


#include "private/moc_kcmloader_p.cpp"
