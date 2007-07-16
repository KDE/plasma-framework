/*
 *   Copyright (C) 2005 by Aaron Seigo <aseigo@kde.org>
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

#ifndef PLASMA_DEFS_H
#define PLASMA_DEFS_H

#include <QtGui/QGraphicsItem>

#include <plasma/plasma_export.h>

/**
 * Namespace for everything in libplasma
 */
namespace Plasma
{

/**
 * The FormFactor enumeration describes how a Plasma::Applet should arrange
 * itself. The value is derived from the container managing the Applet
 * (e.g. in Plasma, a Corona on the desktop or on a panel).
 **/
enum FormFactor { Planar = 0  /**< The applet lives in a plane and has two
                                   degrees of freedom to grow. Optimize for
                                   desktop, laptop or tablet usage: a high
                                   resolution screen 1-3 feet distant from the
                                   viewer. */,
                  MediaCenter /**< As with Planar, the applet lives in a plane
                                   but the interface should be optimized for
                                   medium-to-high resolution screens that are
                                   5-15 feet distant from the viewer. Sometimes
                                   referred to as a "ten foot interface".*/,
                  Horizontal  /**< The applet is constrained vertically, but
                                   can expand horizontally. */,
                  Vertical    /**< The applet is contrained horizontally, but
                                   can expand vertically. */
                };

/**
 * The Direction enumeration describes in which direction, relative to the
 * Applet (and its managing container), popup menus, expanders, balloons,
 * message boxes, arrows and other such visually associated widgets should
 * appear in. This is usually the oposite of the Location.
 **/
enum Direction { Down = 0 /**< Display downards */ ,
                 Up       /**< Display upwards */,
                 Left     /**< Display to the left */,
                 Right    /**< Display to the right */
               };

/**
 * The Location enumeration describes where on screen an element, such as an
 * Applet or its managing container, is positioned on the screen.
 **/
enum Location { Floating = 0 /**< Free floating. Neither geometry or z-ordering
                                  is described precisely by this value. */,
                Desktop      /**< On the planar desktop layer, extending across
                                  the full screen from edge to edge */,
                FullScreen   /**< Full screen */,
                TopEdge      /**< Along the top of the screen*/,
                BottomEdge   /**< Along the bottom of the screen*/,
                LeftEdge     /**< Along the left side of the screen */,
                RightEdge    /**< Along the right side of the screen */
              };

enum ItemTypes { AppletType = QGraphicsItem::UserType + 1,
                 LineEditType = QGraphicsItem::UserType + 2
               };

PLASMA_EXPORT Direction locationToDirection(Location location);

} // Plasma namespace

#endif // multiple inclusion guard
