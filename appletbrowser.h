/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef APPLETBROWSERWINDOW_H_
#define APPLETBROWSERWINDOW_H_

#include <KDE/KDialog>

#include <plasma/plasma_export.h>

namespace Plasma
{

class Corona;
class Containment;

class PLASMA_EXPORT AppletBrowser: public KDialog
{
    Q_OBJECT
public:
    explicit AppletBrowser(Plasma::Corona *corona, QWidget *parent = 0, Qt::WindowFlags f = 0);
    explicit AppletBrowser(Plasma::Containment *containment, QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~AppletBrowser();

protected Q_SLOTS:
    /**
     * Adds currently selected applets
     */
    void addApplet();

    /**
     * Launches a download dialog to retrieve new applets from the Internet
     */
    void downloadApplets();

private:
    void init();
    class Private;
    Private * const d;
};

} // namespace Plasma

#endif /*APPLETBROWSERWINDOW_H_*/
