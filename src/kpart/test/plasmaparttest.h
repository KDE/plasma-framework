/***************************************************************************
 *   Copyright (C) 2014 by Marco Martin <mart@kde.org>                 *
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

#ifndef PLASMAPARTTEST_H
#define PLASMAPARTTEST_H

#include <kparts/mainwindow.h>
#include <kparts/readonlypart.h>

/**
 * This is the application "Shell".  It has a menubar, toolbar, and
 * statusbar but relies on the "Part" to do all the real work.
 *
 * @short Application Shell
 * @author ${AUTHOR} <${EMAIL}>
 * @version ${APP_VERSION}
 */
class PlasmaPartTest : public KParts::MainWindow
{
    Q_OBJECT
public:
    PlasmaPartTest();
    ~PlasmaPartTest();

private:
    KParts::ReadOnlyPart *m_part;
};

#endif // PLASMAPARTTEST_H
