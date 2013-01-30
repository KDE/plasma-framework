/*
 *   Copyright 2007-2008 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2010 Ryan Rix <ry@n.rix.si>
 *   Copyright 2010 Siddharth Sharma <siddharth.kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
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

#ifndef PLASMAKPARTVIEW_H
#define PLASMAKPARTVIEW_H

#include <Plasma/Plasma>
#include <Plasma/View>

namespace Plasma
{
    class Containment;
} // namespace Plasma

class PlasmaKPartView : public Plasma::View
{
Q_OBJECT
public:
    typedef Plasma::ImmutabilityType ImmutabilityType;
    PlasmaKPartView(Plasma::Containment *containment, int uid, QWidget *parent = 0);
    ~PlasmaKPartView();

    /**
     * hook up all needed signals to a containment
     */
    void connectContainment(Plasma::Containment *containment);

    bool autoHide() const;

public Q_SLOTS:
    void setContainment(Plasma::Containment *containment);
    void updateGeometry();

Q_SIGNALS:
    void containmentActivated();
    void autoHideChanged(bool autoHide);
    void geometryChanged();

protected:
    void resizeEvent( QResizeEvent* event );

private:
    bool m_configurationMode;
};

#endif // multiple inclusion guard
