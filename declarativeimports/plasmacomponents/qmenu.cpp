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
#include <QDesktopWidget>
#include <QGraphicsObject>
#include <QGraphicsView>
#include <QDeclarativeItem>

#include "plasmacomponentsplugin.h"
QMenuProxy::QMenuProxy (QObject *parent)
    : QObject(parent),
      m_status(DialogStatus::Closed)
{
    m_menu = new QMenu(0);
    connect(m_menu, SIGNAL(triggered(QAction *)),
            this, SLOT(itemTriggered(QAction *)));
}

QMenuProxy::~QMenuProxy()
{
    delete m_menu;
}

QDeclarativeListProperty<QMenuItem> QMenuProxy::content()
{
    return QDeclarativeListProperty<QMenuItem>(this, m_items);
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
        foreach(QMenuItem* item, m_items) {
            m_menu->addAction(item);
        }
        m_menu->updateGeometry();
    }

    m_visualParent = parent;
    emit visualParentChanged();
}

bool QMenuProxy::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::ChildAdded: {
        QChildEvent *ce = static_cast<QChildEvent *>(event);
        QMenuItem *mi = qobject_cast<QMenuItem *>(ce->child());
        //FIXME: linear complexity here
        if (mi && !m_items.contains(mi)) {
            m_menu->addAction(mi);
            m_items << mi;
        }
        break;
    }

    case QEvent::ChildRemoved: {
        QChildEvent *ce = static_cast<QChildEvent *>(event);
        QMenuItem *mi = qobject_cast<QMenuItem *>(ce->child());

        //FIXME: linear complexity here
        if (mi) {
            m_menu->removeAction(mi);
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
    QMenuItem *item = new QMenuItem(this);
    item->setText(text);
    m_menu->addAction(item);
    m_items << item;
}

void QMenuProxy::addMenuItem(QMenuItem *item)
{
    m_menu->addAction(item);
    m_items << item;
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

void QMenuProxy::open(int x, int y)
{
    m_menu->clear();
    foreach(QMenuItem* item, m_items) {
        m_menu->addAction (item);
    }

    QPoint screenPos;

    QGraphicsObject *parentItem;
    if (m_visualParent) {
        parentItem = qobject_cast<QGraphicsObject *>(m_visualParent.data());
    } else {
        parentItem = qobject_cast<QGraphicsObject *>(parent());
    }

    if (!parentItem || !parentItem->scene()) {
        open(0, 0);
        return;
    }

    QList<QGraphicsView*> views = parentItem->scene()->views();

    if (views.size() < 1) {
        open(0, 0);
        return;
    }

    QGraphicsView *view = 0;
    if (views.size() == 1) {
        view = views[0];
    } else {
        QGraphicsView *found = 0;
        QGraphicsView *possibleFind = 0;
        foreach (QGraphicsView *v, views) {
            if (v->sceneRect().intersects(parentItem->sceneBoundingRect()) ||
                v->sceneRect().contains(parentItem->scenePos())) {
                if (v->isActiveWindow()) {
                    found = v;
                } else {
                    possibleFind = v;
                }
            }
        }
        view = found ? found : possibleFind;
    }

    if (view) {
        screenPos = view->mapToGlobal(view->mapFromScene(parentItem->scenePos()+ QPoint(x, y)));
    } else {
        screenPos = QApplication::activeWindow()->mapToGlobal(QPoint(x, y));
    }

    m_menu->popup(screenPos);
    m_status = DialogStatus::Open;
    emit statusChanged();
}

void QMenuProxy::open()
{
    m_menu->clear();

    foreach(QMenuItem* item, m_items) {
        m_menu->addAction (item);
    }
    m_menu->updateGeometry();

    QGraphicsObject *parentItem;
    if (m_visualParent) {
        parentItem = qobject_cast<QGraphicsObject *>(m_visualParent.data());
    } else {
        parentItem = qobject_cast<QGraphicsObject *>(parent());
    }

    if (!parentItem || !parentItem->scene()) {
        open(0, 0);
        return;
    }

    QList<QGraphicsView*> views = parentItem->scene()->views();

    if (views.size() < 1) {
        open(0, 0);
        return;
    }

    QGraphicsView *view = 0;
    if (views.size() == 1) {
        view = views[0];
    } else {
        QGraphicsView *found = 0;
        QGraphicsView *possibleFind = 0;
        foreach (QGraphicsView *v, views) {
            if (v->sceneRect().intersects(parentItem->sceneBoundingRect()) ||
                v->sceneRect().contains(parentItem->scenePos())) {
                if (v->isActiveWindow()) {
                    found = v;
                } else {
                    possibleFind = v;
                }
            }
        }
        view = found ? found : possibleFind;
    }

    if (!view) {
        open(0, 0);
        return;
    }

    const QRect avail = QApplication::desktop()->availableGeometry(view);
    QPoint menuPos = view->mapToGlobal(view->mapFromScene(parentItem->scenePos()+QPoint(0, parentItem->boundingRect().height())));

    if (menuPos.y() + m_menu->sizeHint().height() > avail.bottom()) {
        menuPos = view->mapToGlobal(view->mapFromScene(parentItem->scenePos() - QPoint(0, m_menu->sizeHint().height())));
    }

    m_menu->popup(menuPos);
    m_status = DialogStatus::Open;
    emit statusChanged();
}

void QMenuProxy::close()
{
    m_menu->hide();
    m_status = DialogStatus::Closed;
    emit statusChanged();
}

#include "qmenu.moc"

