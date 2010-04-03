/*
 *   Copyright (C) 2010 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#ifndef PLASMA_DIALOGMANAGER_H
#define PLASMA_DIALOGMANAGER_H

#include <QtCore/QObject>

#include <plasma/plasma_export.h>


namespace Plasma
{
    class Applet;
    class Corona;

    class AbstractDialogManagerPrivate;

/**
 * @class AbstractDialogManager plasma/dialogmanager.h <Plasma/AbstractDialogManager>
 *
 * @short The AbstractDialogManager class shows the dialogs shown by applets and the rest of the shell.
 *   a AbstractDialogManager can manage aspects like positioning, sizing and widget style 
 *   of dialogs sich as the applet configuration dialog.
 * @since 4.5
 */
class PLASMA_EXPORT AbstractDialogManager : public QObject
{
    Q_OBJECT

public:
    explicit AbstractDialogManager(Plasma::Corona *parent=0);
    ~AbstractDialogManager();

public Q_SLOTS:
    /**
     * This fake virtual slot shows a dialog belonging to an applet.
     * There is no guarantee how the implementation will show it
     * @param widget the dialog widget
     * @param applet the applet that owns the dialog
     */
    void showDialog(QWidget *widget, Plasma::Applet *applet);

private:

    AbstractDialogManagerPrivate * const d;
};

}

#endif
