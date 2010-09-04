/*
*   Copyright 2010 Ryan Rix <ry@n.rix.si>
*   Copyright 2010 Siddharth Sharma <siddharth.kde@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License version 2 as
*   published by the Free Software Foundation
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

#ifndef CONTAINMENTSHELL_H
#define CONTAINMENTSHELL_H

#include "../plasmakpart.h"

#include <kparts/mainwindow.h>
#include <KDialog>

/**
 * This is the application "Shell".  It has a menubar, toolbar, and
 * statusbar but relies on the "Part" to do all the real work.
 *
 * @short Generic Application Shell
 * @author Ryan Rix <ry@n.rix.si>
 * @version 0.01
 */
class ContainmentShell : public KParts::MainWindow
{
Q_OBJECT
public:
    ContainmentShell();
    virtual ~ContainmentShell();

public Q_SLOTS:
    void optionsPreferences();

private:
    KParts::Part* m_part;
    KDialog* m_dialog;
};

#endif // CONTAINMENTSHELL_H
