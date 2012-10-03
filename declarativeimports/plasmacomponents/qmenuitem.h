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

#ifndef QMENUITEM_H
#define QMENUITEM_H

#include <QAction>
#include <QObject>

class QMenuItem : public QAction
{
    Q_OBJECT

    /**
     * The parent object
     */
    Q_PROPERTY(QObject *parent READ parent WRITE setParent)

    /**
     * If true, the menu item will behave like a separator
     */
    Q_PROPERTY(bool separator READ isSeparator WRITE setSeparator)

public:
    QMenuItem(QObject *parent = 0);

Q_SIGNALS:
    void clicked();
};

#endif // QMENUITEM_H

