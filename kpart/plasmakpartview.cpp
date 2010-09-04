/*
 *   Copyright 2007-2008 Aaron Seigo <aseigo@kde.org>
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

#include "plasmakpartview.h"
#include "plasmakpartcorona.h"

#include <Plasma/Applet>
#include <Plasma/Containment>

PlasmaKPartView::PlasmaKPartView(Plasma::Containment* containment, int uid, QWidget* parent)
    : Plasma::View(containment, uid, parent),
      m_configurationMode(false)
{
    setFocusPolicy(Qt::NoFocus);
    connectContainment(containment);

    setWallpaperEnabled( false );
}

PlasmaKPartView::~PlasmaKPartView()
{
}

void PlasmaKPartView::connectContainment(Plasma::Containment* containment)
{
    if (!containment) {
        return;
    }

    connect(this, SIGNAL(sceneRectAboutToChange()), this, SLOT(updateGeometry()));
    connect(containment, SIGNAL(toolBoxVisibilityChanged(bool)), this, SLOT(updateConfigurationMode(bool)));
}

void PlasmaKPartView::setContainment(Plasma::Containment* c)
{
    if (containment()) {
        disconnect(containment(), 0, this, 0);
    }

    Plasma::View::setContainment(c);
    connectContainment(c);
    updateGeometry();
}

void PlasmaKPartView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    updateGeometry();
    emit geometryChanged();
}

void PlasmaKPartView::updateGeometry()
{
    Plasma::Containment* c = containment();
    if (!c) {
        return;
    }

    kDebug() << "New containment geometry is" << c->geometry();

    if (c->size().toSize() != size()) {
        c->setMaximumSize(size());
        c->setMinimumSize(size());
        c->resize(size());
    }
}

#include "plasmakpartview.moc"
