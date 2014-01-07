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
        setVisible(true);
    });
    setAcceptHoverEvents(true);
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

QQuickItem *ToolTip::target() const
{
    if (m_target.data()) {
        return m_target.data();
    } else {
        QQuickItem *qqi = qobject_cast<QQuickItem*>(parent());
        return qqi;
    }
}

void ToolTip::setTarget(QQuickItem *target)
{
    if (m_target.data() == target) {
        return;
    }

    m_target = target;
    setParentItem(target);

    property("anchors").value<QObject *>()->setProperty("fill", QVariant::fromValue(parentItem()));

    emit targetChanged();
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
        dlg->setVisualParent(target());
        dlg->setVisible(true);
    } else {
        dlg->setVisible(false);
    }
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

void ToolTip::hoverEnterEvent(QHoverEvent *event)
{
    if (ToolTipDialog::instance()->isVisible()) {
        //FIXME: setVisible needs to be renamed in sync or something like that
        setVisible(true);
    } else {
        m_showTimer->start(500);
    }

    //relay the event
    if (window()) {
        window()->sendEvent(target(), event);
    }
}

void ToolTip::hoverLeaveEvent(QHoverEvent *event)
{
    m_showTimer->stop();

    //relay the event
    if (window()) {
        window()->sendEvent(target(), event);
    }
}
