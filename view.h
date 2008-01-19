/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
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

#ifndef VIEW_H
#define VIEW_H

#include <QtGui/QGraphicsView>

#include <plasma/plasma_export.h>

namespace Plasma
{

class Containment;
class Corona;

/**
 * @short A QGraphicsView for Plasma::Applets
 */
class PLASMA_EXPORT View : public QGraphicsView
{
    Q_OBJECT

public:
    View(int screen, Corona *corona, QWidget *parent = 0);
    explicit View(Containment *containment, QWidget *parent = 0);
    ~View();

    /**
     * Sets whether or not to draw the containment wallpaper when painting
     * on this item
     */
    void setDrawWallpaper(bool draw);

    /**
     * @return whether or not containments should draw wallpaper
     */
    bool drawWallpaper() const;

    /**
     * Sets which screen this view is associated with, if any.
     * This will also set the containment if a valid screen is specified
     *
     * @arg screen the xinerama screen number; -1 for no screen
     */
    void setScreen(int screen);

    /**
     * Returns the screen this view is associated with
     *
     * @return the xinerama screen number, or -1 for none
     */
    int screen() const;

    /**
     * Sets the containment for this view, which will also cause the view
     * to track the geometry of the containment.
     *
     * @arg containment the containment to center the view on
     */
    void setContainment(Containment *containment);

    /**
     * @return the containment associated with this view, or 0 if none is
     */
    Containment* containment() const;

protected Q_SLOTS:
    void updateSceneRect();

private:
    void initGraphicsView();

    class Private;
    Private * const d;
};

} // namespace Plasma

#endif


