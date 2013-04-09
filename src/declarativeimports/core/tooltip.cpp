/***************************************************************************
 *   Copyright 2011 Marco Martin <mart@kde.org>                            *
 *   Copyright 2011 Artur Duque de Souza <asouza@kde.org>                  *
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "tooltip.h"
#include "tooltipdialog.h"

#include <QQuickItem>
#include <QDebug>

#include "framesvgitem.h"
#include <kwindoweffects.h>

ToolTip::ToolTip(QObject *parent)
    : QObject(parent)
{
}

ToolTip::~ToolTip()
{
}

QQuickItem *ToolTip::mainItem() const
{
    return m_mainItem.data();
}

void ToolTip::setMainItem(QQuickItem *mainItem)
{
    if (m_mainItem.data() != mainItem) {
        if (m_mainItem) {
            m_mainItem.data()->setParent(parent());
        }
        m_mainItem = mainItem;
        emit mainItemChanged();
    }
}

QQuickItem *ToolTip::visualParent() const
{
    if (m_visualParent.data()) {
        return m_visualParent.data();
    } else {
        QQuickItem *qqi = qobject_cast<QQuickItem*>(parent());
        return qqi;
    }
}

void ToolTip::setVisualParent(QQuickItem *visualParent)
{
    if (m_visualParent.data() == visualParent) {
        return;
    }
    emit visualParentChanged();
}

bool ToolTip::isVisible() const
{
    ToolTipDialog *dlg = ToolTipDialog::instance();
    return (dlg->mainItem() == mainItem() && mainItem() && mainItem()->isVisible());
}

void ToolTip::setVisible(const bool visible)
{
    ToolTipDialog *dlg = ToolTipDialog::instance();
    if (visible) {
        dlg->setMainItem(mainItem());
        dlg->setVisualParent(visualParent());
        dlg->setVisible(true);
    } else {
        dlg->setVisible(false);
    }
}

