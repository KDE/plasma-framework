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

#ifndef QMENU_PROXY_H
#define QMENU_PROXY_H

#include <QObject>
#include <QMenu>
#include <QDeclarativeListProperty>
#include "qmenuitem.h"

class QMenuProxy : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QDeclarativeListProperty<QMenuItem> actions READ actions)
    Q_CLASSINFO("DefaultProperty", "actions")

public:
    QMenuProxy(QObject *parent = 0);
    ~QMenuProxy();

    QDeclarativeListProperty<QMenuItem> actions();
    int actionCount() const;
    QMenuItem *action(int) const;

    void showMenu(int x, int y);
    Q_INVOKABLE void open();
    Q_INVOKABLE void close();

private:
    QList<QMenuItem*> m_actions;
    QMenu *m_menu;
};

#endif //QMENU_PROXY_H

