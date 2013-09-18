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
{
    delete m_module;
}

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
    m_pluginName = name;
    checkKCMModule();

    emit pluginNameChanged();
}

void KCMLoader::showEvent(QShowEvent *ev)
{
    checkKCMModule();
    m_module->show();
    m_module->windowHandle()->setParent(window());
}

void KCMLoader::hideEvent(QHideEvent *ev)
{
    if (m_module) {
        m_module->hide();
    }
}

void KCMLoader::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChanged(newGeometry, oldGeometry);

    if (!m_module) {
        return;
    }
    m_module->windowHandle()->setX(mapToScene(QPoint()).x());
    m_module->windowHandle()->setY(mapToScene(QPoint()).y());
    m_module->windowHandle()->setWidth(width());
    m_module->windowHandle()->setHeight(height());
}

void KCMLoader::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (change == QQuickItem::ItemSceneChange) {
        //we have a window: create the 
        if (value.window && m_module) {
            m_module->windowHandle()->setParent(window());
        }
    }
    QQuickItem::itemChange(change, value);
}

void KCMLoader::checkKCMModule()
{
    if (m_module || m_pluginName.isEmpty()) {
        return;
    }

    m_module = new KCModuleProxy(m_pluginName);
    m_module->setAttribute(Qt::WA_TranslucentBackground);
    m_module->load();
    m_module->show();
    m_module->windowHandle()->setParent(window());
    m_module->windowHandle()->setX(mapToScene(QPoint()).x());
    m_module->windowHandle()->setY(mapToScene(QPoint()).y());
    m_module->windowHandle()->setWidth(width());
    m_module->windowHandle()->setHeight(height());
}

#include "private/moc_kcmloader_p.cpp"
