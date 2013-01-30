/*
*   Copyright 2011 by Aaron Seigo <aseigo@kde.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License version 2, 
*   or (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef PLASMA_WINDOWSHADOWS_H
#define PLASMA_WINDOWSHADOWS_H

#include <QSet>

#include <plasma/svg.h>

namespace Plasma
{

class WindowShadows : Plasma::Svg
{
    Q_OBJECT

public:
    static WindowShadows *self();

    explicit WindowShadows(QObject *parent = 0);

    void addWindow(const QWidget *window);
    void removeWindow(const QWidget *window);

private Q_SLOTS:

private:
    class Private;
    Private * const d;

    Q_PRIVATE_SLOT(d, void updateShadows())
    Q_PRIVATE_SLOT(d, void windowDestroyed(QObject *deletedObject))
};

} // namespace Plasma

#endif

