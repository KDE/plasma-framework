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
#include <QTimer>

#include "framesvgitem.h"
#include <kwindoweffects.h>

ToolTip::ToolTip(QObject *parent)
    : QObject(parent)
{
}

ToolTip::~ToolTip()
{
}

QQuickItem *ToolTip::target() const
{
    return m_target.data();
}

void ToolTip::setTarget(QQuickItem *target)
{
    if (m_target.data() != target) {
        m_target = target;
        emit targetChanged();
    }
}

QQmlComponent* ToolTip::mainComponent() const
{
    return m_mainComponent.data();
}

void ToolTip::setMainComponent(QQmlComponent* mainComponent)
{
    m_mainComponent = mainComponent;
}

QQuickItem *ToolTip::mainItem() const
{
    return m_mainItem.data();
}

void ToolTip::setMainItem(QQuickItem *mainItem)
{
    qDebug() << "XXXX mainitem changed: " << mainItem->width() << mainItem->height();

    if (m_mainItem.data() != mainItem) {
        qDebug() << " XXX new mainItem";
//         disconnect(m_mainItem.data(), &QQuickItem::widthChanged, this, &ToolTip::syncGeometry);
//         disconnect(m_mainItem.data(), &QQuickItem::heightChanged, this, &ToolTip::syncGeometry);
        if (m_mainItem) {
            m_mainItem.data()->setParent(parent());
        }
        m_mainItem = mainItem;

        if (mainItem) {
            //mainItem->setParentItem(0);
//             connect(m_mainItem.data(), &QQuickItem::widthChanged, this, &ToolTip::syncGeometry);
//             connect(m_mainItem.data(), &QQuickItem::heightChanged, this, &ToolTip::syncGeometry);
            qDebug() << "XXX new mainITem connected";
            //mainItem->setParent(contentItem());
            //mainItem->setProperty("parent", QVariant::fromValue(contentItem()));

//             if (mainItem->metaObject()->indexOfSignal("widthChanged")) {
//                 connect(mainItem, SIGNAL(widthChanged()), m_syncTimer, SIGNAL(start()));
//             }
//             if (mainItem->metaObject()->indexOfSignal("heightChanged")) {
//                 connect(mainItem, SIGNAL(heightChanged()), m_syncTimer, SIGNAL(start()));
//             }
        }

        //if this is called in Compenent.onCompleted we have to wait a loop the item is added to a scene
        emit mainItemChanged();
    }
}

QQuickItem *ToolTip::visualParent() const
{
    if (m_visualParent.data()) {
        qDebug() << "returning real visualParent";
        return m_visualParent.data();
    } else {
        qDebug() << "returning parent as visualParent";
        QQuickItem *qqi = qobject_cast<QQuickItem*>(parent());
        return qqi;
    }
}

void ToolTip::setVisualParent(QQuickItem *visualParent)
{
    if (m_visualParent.data() == visualParent) {
        return;
    }

    if (visualParent) {
        //setPosition(popupPosition(visualParent, Qt::AlignCenter));
    }
    emit visualParentChanged();
}

bool ToolTip::isVisible() const
{
    //return QQuickWindow::isVisible();
    return true;
}

void ToolTip::setVisible(const bool visible)
{
    ToolTipDialog *dlg = ToolTipDialog::instance();
    qDebug() << "creating tooltipdialog" << visible;
    if (visible) {
        //dlg->mainItem()->deleteLater();

//         QObject *myObject = m_mainComponent.data()->create();
//         QQuickItem *item = qobject_cast<QQuickItem*>(myObject);
        qDebug() << "---- XXX Setting visible: " << mainItem();
        setMainItem(mainItem());


        qDebug() << "XXX showing tooltip: " << ToolTipDialog::instance();
        qDebug() << "XXX positioning near: " << visualParent()->objectName();
        dlg->setMainItem(mainItem());
        dlg->setVisualParent(visualParent());
        dlg->syncToMainItemSize();
        //dlg->setPosition(dlg->popupPosition(visualParent()));
        dlg->setVisible(true);

//         syncGeometry();
//         raise();
    } else {
        dlg->setVisible(false);
        //dlg->mainItem()->deleteLater();
    }
    //QQuickWindow::setVisible(visible);
}

