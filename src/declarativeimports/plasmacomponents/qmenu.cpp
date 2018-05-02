/***************************************************************************
 *   Copyright 2011 Viranch Mehta <viranch.mehta@gmail.com>                *
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

#include "qmenu.h"

#include <QApplication>
#include <QDebug>
#include <QQuickWindow>
#include <QQuickItem>
#include <QScreen>
#include <QTimer>
#include <QVersionNumber>

#include <KAcceleratorManager>

#include "plasmacomponentsplugin.h"
QMenuProxy::QMenuProxy(QObject *parent)
    : QObject(parent),
      m_menu(nullptr),
      m_status(DialogStatus::Closed),
      m_placement(Plasma::Types::LeftPosedTopAlignedPopup)
{
    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        m_menu = new QMenu(0);
        // Breeze and Oxygen have rounded corners on menus. They set this attribute in polish()
        // but at that time the underlying surface has already been created where setting this
        // flag makes no difference anymore (Bug 385311)
        m_menu->setAttribute(Qt::WA_TranslucentBackground);

        KAcceleratorManager::manage(m_menu);
        connect(m_menu, &QMenu::triggered, this, &QMenuProxy::itemTriggered);
        connect(m_menu, &QMenu::aboutToHide, [ = ]() {
                m_status = DialogStatus::Closed;
                emit statusChanged();
        });
    }
}

QMenuProxy::~QMenuProxy()
{
    delete m_menu;
}

QQmlListProperty<QMenuItem> QMenuProxy::content()
{
    return QQmlListProperty<QMenuItem>(this, m_items);
}

int QMenuProxy::actionCount() const
{
    return m_items.count();
}

QMenuItem *QMenuProxy::action(int index) const
{
    return m_items.at(index);
}

DialogStatus::Status QMenuProxy::status() const
{
    return m_status;
}

QObject *QMenuProxy::visualParent() const
{
    return m_visualParent.data();
}

void QMenuProxy::setVisualParent(QObject *parent)
{
    if (m_visualParent.data() == parent) {
        return;
    }

    //if the old parent was a QAction, disconnect the menu from it
    QAction *action = qobject_cast<QAction *>(m_visualParent.data());
    if (action) {
        action->setMenu(0);
        m_menu->clear();
    }
    //if parent is a QAction, become a submenu
    action = qobject_cast<QAction *>(parent);
    if (action) {
        action->setMenu(m_menu);
        m_menu->clear();
        foreach (QMenuItem *item, m_items) {
            if (item->section()) {
                if (!item->isVisible()) {
                    continue;
                }

                m_menu->addSection(item->text());
            } else {
                m_menu->addAction(item->action());
            }
        }
        m_menu->updateGeometry();
    }

    m_visualParent = parent;
    emit visualParentChanged();
}

QWindow *QMenuProxy::transientParent()
{
    if (!m_menu) {
        return nullptr;
    }
    return m_menu->windowHandle()->transientParent();
}

void QMenuProxy::setTransientParent(QWindow *parent)
{
    if (parent == m_menu->windowHandle()->transientParent()) {
        return;
    }

    m_menu->windowHandle()->setTransientParent(parent);
    emit transientParentChanged();
}

Plasma::Types::PopupPlacement QMenuProxy::placement() const
{
    return m_placement;
}

void QMenuProxy::setPlacement(Plasma::Types::PopupPlacement placement)
{
    if (m_placement != placement) {
        m_placement = placement;

        emit placementChanged();
    }
}

int QMenuProxy::minimumWidth() const
{
    return m_menu->minimumWidth();
}

void QMenuProxy::setMinimumWidth(int width)
{
    if (m_menu->minimumWidth() != width) {
        m_menu->setMinimumWidth(width);

        emit minimumWidthChanged();
    }
}

int QMenuProxy::maximumWidth() const
{
    return m_menu->maximumWidth();
}

void QMenuProxy::setMaximumWidth(int width)
{
    if (m_menu->maximumWidth() != width) {
        m_menu->setMaximumWidth(width);

        emit maximumWidthChanged();
    }
}

void QMenuProxy::resetMaximumWidth()
{
    setMaximumWidth(QWIDGETSIZE_MAX);
}

bool QMenuProxy::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::ChildAdded: {
        QChildEvent *ce = static_cast<QChildEvent *>(event);
        QMenuItem *mi = qobject_cast<QMenuItem *>(ce->child());
        //FIXME: linear complexity here
        if (mi && !m_items.contains(mi)) {
            if (mi->separator()) {
                m_menu->addSection(mi->text());
            } else {
                m_menu->addAction(mi->action());
            }
            m_items << mi;
        }
        break;
    }

    case QEvent::ChildRemoved: {
        QChildEvent *ce = static_cast<QChildEvent *>(event);
        QMenuItem *mi = qobject_cast<QMenuItem *>(ce->child());

        //FIXME: linear complexity here
        if (mi) {
            m_menu->removeAction(mi->action());
            m_items.removeAll(mi);
        }
        break;
    }

    default:
        break;
    }

    return QObject::event(event);
}

void QMenuProxy::clearMenuItems()
{
    qDeleteAll(m_items);
    m_items.clear();
}

void QMenuProxy::addMenuItem(const QString &text)
{
    QMenuItem *item = new QMenuItem();
    item->setText(text);
    m_menu->addAction(item->action());
    m_items << item;
}

void QMenuProxy::addMenuItem(QMenuItem *item, QMenuItem *before)
{
    if (before) {
        if (m_items.contains(item)) {
            m_menu->removeAction(item->action());
            m_items.removeAll(item);
        }

        m_menu->insertAction(before->action(), item->action());

        const int index = m_items.indexOf(before);

        if (index != -1) {
            m_items.insert(index, item);
        } else {
            m_items << item;
        }

    } else if (!m_items.contains(item)) {
        m_menu->addAction(item->action());
        m_items << item;
    }
}

void QMenuProxy::addSection(const QString &text)
{
    m_menu->addSection(text);
}

void QMenuProxy::removeMenuItem(QMenuItem *item)
{
    if (!item) {
        return;
    }

    m_menu->removeAction(item->action());
    m_items.removeOne(item);
}

void QMenuProxy::itemTriggered(QAction *action)
{
    QMenuItem *item = qobject_cast<QMenuItem *>(action);
    if (item) {
        emit triggered(item);
        int index = m_items.indexOf(item);
        if (index > -1) {
            emit triggeredIndex(index);
        }
    }
}

void QMenuProxy::rebuildMenu()
{
    m_menu->clear();

    foreach (QMenuItem *item, m_items) {
        if (item->section()) {
            if (!item->isVisible()) {
                continue;
            }

            m_menu->addSection(item->text());
        } else {
            m_menu->addAction(item->action());
            if (item->action()->menu()) {
                //This ensures existence of the QWindow
                m_menu->winId();
                item->action()->menu()->winId();
                item->action()->menu()->windowHandle()->setTransientParent(m_menu->windowHandle());
            }
        }
    }

    m_menu->adjustSize();
}

void QMenuProxy::open(int x, int y)
{
    qDebug() << "Opening menu at" << x << y;

    QQuickItem *parentItem = nullptr;

    if (m_visualParent) {
        parentItem = qobject_cast<QQuickItem *>(m_visualParent.data());
    } else {
        parentItem = qobject_cast<QQuickItem *>(parent());
    }

    if (!parentItem) {
        return;
    }

    rebuildMenu();

    QPointF pos = parentItem->mapToScene(QPointF(x, y));

    if (parentItem->window() && parentItem->window()->screen()) {
        pos = parentItem->window()->mapToGlobal(pos.toPoint());
    }

    openInternal(pos.toPoint());
}

Q_INVOKABLE void QMenuProxy::openRelative()
{
    QQuickItem *parentItem = nullptr;

    if (m_visualParent) {
        parentItem = qobject_cast<QQuickItem *>(m_visualParent.data());
    } else {
        parentItem = qobject_cast<QQuickItem *>(parent());
    }

    if (!parentItem) {
        return;
    }

    rebuildMenu();

    QPointF pos;

    using namespace Plasma;

    auto boundaryCorrection = [&pos, this, parentItem](int hDelta, int vDelta) {
        if (!parentItem->window()) {
            return;
        }
        QScreen *screen = parentItem->window()->screen();
        if (!screen) {
            return;
        }
        QRect geo = screen->geometry();
        pos = parentItem->window()->mapToGlobal(pos.toPoint());

        if (pos.x() < geo.x()) {
            pos.setX(pos.x() + hDelta);
        }
        if (pos.y() < geo.y()) {
            pos.setY(pos.y() + vDelta);
        }

        if (geo.x() + geo.width() < pos.x() + this->m_menu->width()) {
            pos.setX(pos.x() + hDelta);
        }
        if (geo.y() + geo.height() < pos.y() + this->m_menu->height()) {
            pos.setY(pos.y() + vDelta);
        }
    };

    switch(m_placement) {
        case Types::TopPosedLeftAlignedPopup: {
            pos = parentItem->mapToScene(QPointF(0, -m_menu->height()));
            boundaryCorrection(-m_menu->width() + parentItem->width(), m_menu->height() + parentItem->height());
            break;
        }
        case Types::LeftPosedTopAlignedPopup: {
            pos = parentItem->mapToScene(QPointF(-m_menu->width(), 0));
            boundaryCorrection(m_menu->width() + parentItem->width(), -m_menu->height() + parentItem->height());
            break;
        }
        case Types::TopPosedRightAlignedPopup:
            pos = parentItem->mapToScene(QPointF(parentItem->width() - m_menu->width(), -m_menu->height()));
            boundaryCorrection(m_menu->width() - parentItem->width(), m_menu->height() + parentItem->height());
            break;
        case Types::RightPosedTopAlignedPopup: {
            pos = parentItem->mapToScene(QPointF(parentItem->width(), 0));
            boundaryCorrection(-m_menu->width() - parentItem->width(), -m_menu->height() + parentItem->height());
            break;
        }
        case Types::LeftPosedBottomAlignedPopup:
            pos = parentItem->mapToScene(QPointF(-m_menu->width(), -m_menu->height() + parentItem->height()));
            boundaryCorrection(m_menu->width() + parentItem->width(), m_menu->height() - parentItem->height());
            break;
        case Types::BottomPosedLeftAlignedPopup: {
            pos = parentItem->mapToScene(QPointF(0, parentItem->height()));
            boundaryCorrection(-m_menu->width() + parentItem->width(), -m_menu->height() - parentItem->height());
            break;
        }
        case Types::BottomPosedRightAlignedPopup: {
            pos = parentItem->mapToScene(QPointF(parentItem->width() - m_menu->width(), parentItem->height()));
            boundaryCorrection(m_menu->width() - parentItem->width(), -m_menu->height() - parentItem->height());
            break;
        }
        case Types::RightPosedBottomAlignedPopup: {
            pos = parentItem->mapToScene(QPointF(parentItem->width(), -m_menu->height() + parentItem->height()));
            boundaryCorrection(-m_menu->width() - parentItem->width(), m_menu->height() - parentItem->height());
            break;
        }
        default:
            open();
            return;
    }

    openInternal(pos.toPoint());
}

void QMenuProxy::openInternal(QPoint pos)
{
    QQuickItem *parentItem = this->parentItem();

    if (parentItem && parentItem->window()) {
        //create the QWindow
        m_menu->winId();
        m_menu->windowHandle()->setTransientParent(parentItem->window());

        // Workaround for QTBUG-59044
        auto ungrabMouseHack = [this]() {
            QQuickItem *parentItem = this->parentItem();
            if (parentItem && parentItem->window() && parentItem->window()->mouseGrabberItem()) {
                parentItem->window()->mouseGrabberItem()->ungrabMouse();
            }
        };

        //pre 5.8.0 QQuickWindow code is "item->grabMouse(); sendEvent(item, mouseEvent)"
        //post 5.8.0 QQuickWindow code is sendEvent(item, mouseEvent); item->grabMouse()
        if (QVersionNumber::fromString(QString::fromLatin1(qVersion())) > QVersionNumber(5, 8, 0)) {
            QTimer::singleShot(0, this, ungrabMouseHack);
        } else {
            ungrabMouseHack();
        }
        //end workaround
    }

    m_menu->popup(pos);
    m_status = DialogStatus::Open;
    emit statusChanged();
}

QQuickItem *QMenuProxy::parentItem() const
{
    if (m_visualParent) {
        return qobject_cast<QQuickItem *>(m_visualParent.data());
    }

    return qobject_cast<QQuickItem *>(parent());
}

void QMenuProxy::close()
{
    m_menu->hide();
}


