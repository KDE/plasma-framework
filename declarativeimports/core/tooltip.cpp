/***************************************************************************
 *   Copyright 2011 Marco Martin <mart@kde.org>                            *
 *   Copyright 2011 Artur Duque de Souza <asouza@kde.org>                  *
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
#include "declarativeitemcontainer_p.h"

#include <QDeclarativeItem>
#include <QGraphicsObject>
#include <QGraphicsWidget>
#include <QGraphicsScene>
#include <QDebug>
#include <QTimer>

#include <KIcon>
#include <KIconLoader>
#include <Plasma/ToolTipContent>
#include <Plasma/ToolTipManager>


ToolTipProxy::ToolTipProxy(QObject *parent)
    : QObject(parent), m_mainText(""), m_subText(""), m_widget(0)
{
    connect(this, SIGNAL(targetChanged()), this, SLOT(updateToolTip()));
    connect(this, SIGNAL(mainTextChanged()), this, SLOT(updateToolTip()));
    connect(this, SIGNAL(subTextChanged()), this, SLOT(updateToolTip()));
    connect(this, SIGNAL(imageChanged()), this, SLOT(updateToolTip()));
}

ToolTipProxy::~ToolTipProxy()
{
}

QGraphicsObject *ToolTipProxy::target() const
{
    return m_target.data();
}

void ToolTipProxy::setTarget(QGraphicsObject *target)
{
    if (m_target.data() != target) {
        m_target = target;

        m_widget = qobject_cast<QGraphicsWidget*>(m_target.data());
        if (!m_widget) {
            // if this is called in Compenent.onCompleted we have to
            // wait a loop for the item to be added to a scene
            QTimer::singleShot(0, this, SLOT(syncTarget()));
            return;
        }
        emit targetChanged();
    }
}

void ToolTipProxy::syncTarget()
{
    if (!m_target) {
        return;
    }
    // find the scene
    QGraphicsScene *scene = m_target.data()->scene();
    if (!scene) {
        QObject *parent = m_target.data();
        while ((parent = parent->parent())) {
            QGraphicsObject *qo = qobject_cast<QGraphicsObject*>(parent);
            if (qo && qo->scene()) {
                scene = qo->scene();
                scene->addItem(m_target.data());
                break;
            }
        }
    }

    QDeclarativeItem *item = qobject_cast<QDeclarativeItem*>(m_target.data());
    if (!item) {
        return;
    }

    if (!m_declarativeItemContainer) {
	    m_declarativeItemContainer = QWeakPointer<DeclarativeItemContainer>(new DeclarativeItemContainer());
        m_declarativeItemContainer.data()->setObjectName("DIContainer");
        scene->addItem(m_declarativeItemContainer.data());
    } 
    
    if (m_declarativeItemContainer) {
        m_target.data()->setObjectName("Original Item");
        m_declarativeItemContainer.data()->setDeclarativeItem(item, false);
        m_declarativeItemContainer.data()->setAcceptHoverEvents(true);
        m_declarativeItemContainer.data()->setParentItem(m_target.data());
        m_widget = m_declarativeItemContainer.data();
        emit targetChanged();
	}
}

QString ToolTipProxy::mainText() const
{
    return m_mainText;
}

void ToolTipProxy::setMainText(const QString &text)
{
    if (text == m_mainText) {
        return;
    }

    m_mainText = text;
    emit mainTextChanged();
}

QString ToolTipProxy::subText() const
{
    return m_subText;
}

void ToolTipProxy::setSubText(const QString &text)
{
    if (text == m_subText) {
        return;
    }

    m_subText = text;
    emit subTextChanged();
}

// ###TODO: SHOULD BE PIXMAP OR QICON??
QString ToolTipProxy::image() const
{
    return m_image;
}

void ToolTipProxy::setImage(const QString &name)
{
    if (name == m_image) {
        return;
    }

    m_image = name;
    emit imageChanged();
}

void ToolTipProxy::updateToolTip()
{
    if (!m_widget) {
        return;
    }

    Plasma::ToolTipContent data;
    data.setMainText(m_mainText);
    data.setSubText(m_subText);
    data.setImage(KIcon(m_image).pixmap(IconSize(KIconLoader::Desktop)));
    Plasma::ToolTipManager::self()->setContent(m_widget, data);
}

#include "tooltip.moc"

