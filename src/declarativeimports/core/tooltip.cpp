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

#include <QQmlEngine>
#include <QQuickItem>
#include <QDebug>

#include "framesvgitem.h"
#include <kwindoweffects.h>

ToolTip::ToolTip(QQuickItem *parent)
    : QQuickItem(parent)
{
    m_showTimer = new QTimer(this);
    m_showTimer->setSingleShot(true);
    connect(m_showTimer, &QTimer::timeout, [=]() {
        showToolTip();
    });

    setAcceptHoverEvents(true);
    setFiltersChildMouseEvents(true);
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
        m_mainItem = mainItem;

        emit mainItemChanged();
    }
}

void ToolTip::showToolTip()
{
    ToolTipDialog *dlg = ToolTipDialog::instance();

    if (!mainItem()) {
        setMainItem(dlg->loadDefaultItem());
    }

    if (dlg->mainItem()) {
        dlg->mainItem()->setVisible(false);
    }

    if (mainItem()) {
        mainItem()->setProperty("toolTip", QVariant::fromValue(this));
        mainItem()->setVisible(true);
    }

    dlg->setMainItem(mainItem());

    //heuristics for knowing the diration
    Plasma::Types::Direction dir = Plasma::Types::Up;
    QPoint pos = mapToScene(QPoint(0, 0)).toPoint();

    if (window() && window()->screen()) {
        pos = window()->mapToGlobal(pos);
    }
    QPoint popupPos = dlg->popupPosition(this, Qt::AlignCenter);

    if (pos.y() + height() <= popupPos.y()) {
        dir = Plasma::Types::Down;
    } else if (pos.x() + width() <= popupPos.x()) {
        dir = Plasma::Types::Right;
    } else if (pos.y() >= popupPos.y() + dlg->height()) {
        dir = Plasma::Types::Up;
    } else if (pos.x() >= popupPos.x() + dlg->width()) {
        dir = Plasma::Types::Left;
    }

    dlg->setDirection(dir);
    dlg->setVisible(true);
    dlg->setVisualParent(this);
}

QString ToolTip::mainText() const
{
    return m_mainText;
}

void ToolTip::setMainText(const QString &mainText)
{
    if (mainText == m_mainText) {
        return;
    }

    m_mainText = mainText;
    emit mainTextChanged();
}

QString ToolTip::subText() const
{
    return m_subText;
}

void ToolTip::setSubText(const QString &subText)
{
    if (subText == m_subText) {
        return;
    }

    m_subText = subText;
    emit subTextChanged();
}

QVariant ToolTip::icon() const
{
    if (m_icon.isValid()) {
        return m_icon;
    } else {
        return QString();
    }
}

void ToolTip::setIcon(const QVariant &icon)
{
    if (icon == m_icon) {
        return;
    }

    m_icon = icon;
    emit iconChanged();
}

QVariant ToolTip::image() const
{
    if (m_image.isValid()) {
        return m_image;
    } else {
        return QString();
    }
}

void ToolTip::setImage(const QVariant &image)
{
    if (image == m_image) {
        return;
    }

    m_image = image;
    emit imageChanged();
}

bool ToolTip::containsMouse() const
{
    return m_containsMouse;
}

void ToolTip::setContainsMouse(bool contains)
{
    if (m_containsMouse != contains) {
        m_containsMouse = contains;
        emit containsMouseChanged();
    }
    if (!contains) {
        ToolTipDialog::instance()->dismiss();
    }
}

void ToolTip::hoverEnterEvent(QHoverEvent *event)
{
    setContainsMouse(true);
    //m_showTimer->stop();
    if (ToolTipDialog::instance()->isVisible()) {
        // We signal the tooltipmanager that we're "potentially interested,
        // and ask to keep it open for a bit, so other items get the chance
        // to update the content before the tooltip hides -- this avoids
        // flickering
        ToolTipDialog::instance()->keepalive();
        //FIXME: showToolTip needs to be renamed in sync or something like that
        showToolTip();
    } else {
        m_showTimer->start(500);
    }
}

void ToolTip::hoverLeaveEvent(QHoverEvent *event)
{
    setContainsMouse(false);
    m_showTimer->stop();
}

bool ToolTip::childMouseEventFilter(QQuickItem *item, QEvent *event)
{
    return QQuickItem::childMouseEventFilter(item, event);
}

