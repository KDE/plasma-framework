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
#include <QGraphicsObject>
#include <QGraphicsView>

QMenuProxy::QMenuProxy (QObject *parent)
    : QObject (parent)
{
    m_menu = new QMenu(0);
}

QMenuProxy::~QMenuProxy()
{
    delete m_menu;
}

QDeclarativeListProperty<QMenuItem> QMenuProxy::actions()
{
    return QDeclarativeListProperty<QMenuItem>(this, m_actions);
}

int QMenuProxy::actionCount() const
{
    return m_actions.count();
}

QMenuItem *QMenuProxy::action(int index) const
{
    return m_actions.at(index);
}

void QMenuProxy::showMenu(int x, int y)
{
    m_menu->clear();
    foreach(QMenuItem* item, m_actions) {
        m_menu->addAction (item);
    }

    QPoint screenPos = QApplication::activeWindow()->mapToGlobal(QPoint(x, y));
    m_menu->popup(screenPos);
}

void QMenuProxy::open()
{
    QGraphicsObject *parentItem = qobject_cast<QGraphicsObject *>(parent());

    if (!parentItem || !parentItem->scene()) {
        showMenu(0, 0);
        return;
    }

    QList<QGraphicsView*> views = parentItem->scene()->views();

    if (views.size() < 1) {
        showMenu(0, 0);
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
        showMenu(0, 0);
        return;
    }

    QPoint viewPos = view->mapFromScene(parentItem->scenePos());
    showMenu(viewPos.x(), viewPos.y());
}

void QMenuProxy::close()
{
    m_menu->hide();
}

#include "qmenu.moc"

