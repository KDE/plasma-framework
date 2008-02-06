/*
 *   Copyright 2005 by Aaron Seigo <aseigo@kde.org>
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

#ifndef PLASMA_DEFS_H
#define PLASMA_DEFS_H

#include <QtGui/QGraphicsItem>
#include <QtGui/QPainterPath>

#include <plasma/plasma_export.h>

/**
 * Namespace for everything in libplasma
 */
namespace Plasma
{

/**
 * The Constraint enumeration lists the various constraints that Plasma
 * objects have managed for them and which they may wish to react to,
 * for instance in Applet::constraintsUpdated
 */
enum Constraint { NoConstraint = 0,
                  FormFactorConstraint = 1 /** The FormFactor for an object */,
                  LocationConstraint = 2 /** The Location of an object */,
                  ScreenConstraint = 4 /** Which screen an object is on */,
                  SizeConstraint = 8 /** the size of the applet was changed */,
                  ImmutableConstraint = 16 /** the immutability (locked) nature of the applet changed  */,
                  StartupCompletedConstraint = 32 /** application startup has completed */,
                  AllConstraints = FormFactorConstraint | LocationConstraint | ScreenConstraint | SizeConstraint | ImmutableConstraint
};
Q_DECLARE_FLAGS(Constraints, Constraint)

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
                  Vertical    /**< The applet is constrained horizontally, but
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

/**
 * The position enumeration
 *
 **/
enum Position { LeftPositioned    /**< Positioned left */,
                RightPositioned   /**< Positioned right */,
                TopPositioned     /**< Positioned top */, 
                BottomPositioned  /**< Positioned bottom */,
                CenterPositioned  /**< Positioned in the center */
              };

/**
 * Flip enumeration
 */
enum FlipDirection { NoFlip = 0          /**< Do not flip */,
                     HorizontalFlip = 1   /**< Flip horizontally */,
                     VerticalFlip = 2    /**< Flip vertically */
                   };
Q_DECLARE_FLAGS(Flip, FlipDirection)

/**
 * Zoom levels that Plasma is aware of...
 **/
enum ZoomLevel { DesktopZoom = 0 /**< Normal desktop usage, plasmoids are painted normally and have full interaction */,
                 GroupZoom /**< Plasmoids are shown as icons in visual groups; drag and drop and limited context menu interaction only */ ,
                 OverviewZoom /**< Groups become icons themselves */
               };

/**
 * Possible timing alignments
 **/
enum IntervalAlignment { NoAlignment = 0,
                         AlignToMinute,
                         AlignToHour };


enum ItemTypes { AppletType = QGraphicsItem::UserType + 1,
                 LineEditType = QGraphicsItem::UserType + 2
               };

/**
 * The ComonentType enumeration refers to the various types of components,
 * or plugins, supported by plasma.
 */
enum ComponentType { AppletComponent = 1 /**< Plasma::Applet based plugins **/,
                     DataEngineComponent = 2 /**< Plasma::DataEngine based plugins **/,
                     RunnerComponent = 4 /**< Plasma::AbstractRunner based plugsin **/,
                     AnimatorComponent = 8 /**< Plasma::Animator based plugins **/,
                     ContainmentComponent = 16 /**< Plasma::Containment based plugins **/
                   };
Q_DECLARE_FLAGS(ComponentTypes, ComponentType)

enum MarginEdge { TopMargin = 0,
                  BottomMargin,
                  LeftMargin,
                  RightMargin
                };

/**
 * @return the scaling factor (0..1) for a ZoomLevel
 **/
PLASMA_EXPORT qreal scalingFactor(ZoomLevel level);

/**
 * Converts a location to a direction. Handy for figuring out which way to send a popup based on
 * location or to point arrows and other directional items.
 *
 * @param location the location of the container the element will appear in
 * @reutrn the visual direction of the element should be oriented in
 **/
PLASMA_EXPORT Direction locationToDirection(Location location);

/**
 * Returns a nicely rounded rectanglular path for painting.
 */
PLASMA_EXPORT QPainterPath roundedRectangle(const QRectF& rect, qreal radius);

} // Plasma namespace

Q_DECLARE_OPERATORS_FOR_FLAGS(Plasma::Constraints)
Q_DECLARE_OPERATORS_FOR_FLAGS(Plasma::Flip)
Q_DECLARE_OPERATORS_FOR_FLAGS(Plasma::ComponentTypes)

#endif // multiple inclusion guard
