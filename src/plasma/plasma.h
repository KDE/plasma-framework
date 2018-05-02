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

/** @header plasma/plasma.h <Plasma/Plasma> */

#include <QObject>

#include <plasma/plasma_export.h>

class QAction;

/**
 * Namespace for everything in libplasma
 */
namespace Plasma
{
/**
 * @class Types
 * @short Enums and constants used in Plasma
 *
 */
class PLASMA_EXPORT Types : public QObject
{
    Q_OBJECT

public:
    ~Types();
    /**
     * The Constraint enumeration lists the various constraints that Plasma
     * objects have managed for them and which they may wish to react to,
     * for instance in Applet::constraintsUpdated
     */
    enum Constraint {
        NoConstraint = 0, /**< No constraint; never passed in to Applet::constraintsEvent on its own */
        FormFactorConstraint = 1, /**< The FormFactor for an object */
        LocationConstraint = 2, /**< The Location of an object */
        ScreenConstraint = 4, /**< Which screen an object is on */
        ImmutableConstraint = 8, /**< the immutability (locked) nature of the applet changed  */
        StartupCompletedConstraint = 16, /**< application startup has completed */
        ContextConstraint = 32, /**< the context (e.g. activity) has changed */
        UiReadyConstraint = 64, /**< The ui has been completely loaded */ // (FIXME: merged with StartupCompletedConstraint?)
        AllConstraints = FormFactorConstraint | LocationConstraint | ScreenConstraint |
                         ImmutableConstraint
    };
    Q_ENUM(Constraint)
    Q_DECLARE_FLAGS(Constraints, Constraint)

    /**
     * The FormFactor enumeration describes how a Plasma::Applet should arrange
     * itself. The value is derived from the container managing the Applet
     * (e.g. in Plasma, a Corona on the desktop or on a panel).
     **/
    enum FormFactor {
        Planar = 0,  /**< The applet lives in a plane and has two
                    degrees of freedom to grow. Optimize for
                    desktop, laptop or tablet usage: a high
                    resolution screen 1-3 feet distant from the
                    viewer. */
        MediaCenter, /**< As with Planar, the applet lives in a plane
                    but the interface should be optimized for
                    medium-to-high resolution screens that are
                    5-15 feet distant from the viewer. Sometimes
                    referred to as a "ten foot interface".*/
        Horizontal,  /**< The applet is constrained vertically, but
                    can expand horizontally. */
        Vertical,     /**< The applet is constrained horizontally, but
                    can expand vertically. */
        Application /**< The Applet lives in a plane and should be optimized to look as a full application,
                     for the desktop or the particular device. */
    };
    Q_ENUM(FormFactor)

    /**
     * This enumeration describes the type of the Containment.
     * DesktopContainments represent main containments that will own a screen in a mutually exclusive fashion,
     * while PanelContainments are accessories which can be present multiple per screen.
     */
    enum ContainmentType {
        NoContainmentType = -1,  /**< @internal */
        DesktopContainment = 0,  /**< A desktop containment */
        PanelContainment,        /**< A desktop panel */
        CustomContainment = 127, /**< A containment that is neither a desktop nor a panel
                                but something application specific */
        CustomPanelContainment = 128, /**< A customized desktop panel */
        CustomEmbeddedContainment = 129 /**< A customized containment embedded in another applet */
    };
    Q_ENUM(ContainmentType)

    /**
     * A descriptrive type for QActions, to help categorizing them when presented to the user
     */
    enum ActionType {
        AddAction = 0,   /**< The action will cause something new being created*/
        ConfigureAction = 100,   /**< The Action will make some kind of configuration ui to appear */
        ControlAction = 200,  /**< Generic control, similar to ConfigureAction TODO: better doc */
        MiscAction = 300,  /**< A type of action that doesn't fit in the oher categories */
        DestructiveAction = 400,  /**< A dangerous action, such as deletion of objects, plasmoids and files. They are intended to be shown separed from other actions */
        UserAction = DestructiveAction + 1000 /**< If new types are needed in a C++ implementation, define them as ids more than  UserAction*/
    };
    Q_ENUM(ActionType)

    /**
     * The Direction enumeration describes in which direction, relative to the
     * Applet (and its managing container), popup menus, expanders, balloons,
     * message boxes, arrows and other such visually associated widgets should
     * appear in. This is usually the oposite of the Location.
     **/
    enum Direction {
        Down = 0, /**< Display downards */
        Up,       /**< Display upwards */
        Left,     /**< Display to the left */
        Right     /**< Display to the right */
    };
    Q_ENUM(Direction)

    /**
     * The Location enumeration describes where on screen an element, such as an
     * Applet or its managing container, is positioned on the screen.
     **/
    enum Location {
        Floating = 0, /**< Free floating. Neither geometry or z-ordering
                     is described precisely by this value. */
        Desktop,      /**< On the planar desktop layer, extending across
                     the full screen from edge to edge */
        FullScreen,   /**< Full screen */
        TopEdge,      /**< Along the top of the screen*/
        BottomEdge,   /**< Along the bottom of the screen*/
        LeftEdge,     /**< Along the left side of the screen */
        RightEdge     /**< Along the right side of the screen */
    };
    Q_ENUM(Location)

    /**
     * The position enumeration
     *
     **/
    enum Position {
        LeftPositioned,    /**< Positioned left */
        RightPositioned,   /**< Positioned right */
        TopPositioned,     /**< Positioned top */
        BottomPositioned,  /**< Positioned bottom */
        CenterPositioned   /**< Positioned in the center */
    };
    Q_ENUM(Position)

    /**
     * The popup position enumeration relatively to his attached widget
     *
     **/
    enum PopupPlacement {
        FloatingPopup = 0,            /**< Free floating, non attached popup */
        TopPosedLeftAlignedPopup,     /**< Popup positioned on the top, aligned
                                     to the left of the wigdet */
        TopPosedRightAlignedPopup,    /**< Popup positioned on the top, aligned
                                     to the right of the widget */
        LeftPosedTopAlignedPopup,     /**< Popup positioned on the left, aligned
                                     to the top of the wigdet */
        LeftPosedBottomAlignedPopup,  /**< Popup positioned on the left, aligned
                                     to the bottom of the widget */
        BottomPosedLeftAlignedPopup,  /**< Popup positioned on the bottom, aligned
                                     to the left of the wigdet */
        BottomPosedRightAlignedPopup, /**< Popup positioned on the bottom, aligned
                                     to the right of the widget */
        RightPosedTopAlignedPopup,    /**< Popup positioned on the right, aligned
                                     to the top of the wigdet */
        RightPosedBottomAlignedPopup  /**< Popup positioned on the right, aligned
                                     to the bottom of the widget */
    };
    Q_ENUM(PopupPlacement)

    /**
     * Flip enumeration
     */
    enum FlipDirection {
        NoFlip = 0,          /**< Do not flip */
        HorizontalFlip = 1,  /**< Flip horizontally */
        VerticalFlip = 2     /**< Flip vertically */
    };
    Q_ENUM(FlipDirection)
    Q_DECLARE_FLAGS(Flip, FlipDirection)

    /**
     * Possible timing alignments
     **/
    enum IntervalAlignment {
        NoAlignment = 0, /**< No alignment **/
        AlignToMinute, /**< Align to the minute **/
        AlignToHour /**< Align to the hour **/
    };
    Q_ENUM(IntervalAlignment)

    /**
     * Defines the immutability of items like applets, corona and containments
     * they can be free to modify, locked down by the user or locked down by the
     * system (e.g. kiosk setups).
     */
    enum ImmutabilityType {
        Mutable = 1,        /**< The item can be modified in any way **/
        UserImmutable = 2,  /**< The user has requested a lock down, and can undo
                           the lock down at any time **/
        SystemImmutable = 4 /**<  the item is locked down by the system, the user
                           can't unlock it **/
    };
    Q_ENUM(ImmutabilityType)

    /**
     * The ComonentType enumeration refers to the various types of components,
     * or plugins, supported by plasma.
     */
    enum ComponentType {
        AppletComponent = 1,      /**< Plasma::Applet based plugins **/
        DataEngineComponent = 2,  /**< Plasma::DataEngine based plugins **/
        ContainmentComponent = 4,/**< Plasma::Containment based plugins **/
        WallpaperComponent = 8,   /**< Plasma::Wallpaper based plugins **/
        GenericComponent = 16      /** Generic repositories of files, usually they keep QML files and their assets **/
    };
    Q_ENUM(ComponentType)
    Q_DECLARE_FLAGS(ComponentTypes, ComponentType)

    enum MarginEdge {
        TopMargin = 0, /**< The top margin **/
        BottomMargin, /**< The bottom margin **/
        LeftMargin, /**< The left margin **/
        RightMargin /**< The right margin **/
    };
    Q_ENUM(MarginEdge)

    /**
     * Status of an applet
     * @since 4.3
     */
    enum ItemStatus {
        UnknownStatus = 0, /**< The status is unknown **/
        PassiveStatus = 1, /**< The Item is passive **/
        ActiveStatus = 2, /**< The Item is active **/
        NeedsAttentionStatus = 3, /**< The Item needs attention **/
        RequiresAttentionStatus = 4, /**< The Item needs persistent attention **/
        AcceptingInputStatus = 5, /**< The Item is accepting input **/
        //FIXME KF6: this should be the smallest status
        HiddenStatus = 6    /**< The Item will be hidden totally  **/
    };
    Q_ENUM(ItemStatus)

    enum TrustLevel {
        UnverifiableTrust = 0,      /**< The trust of the object can not be verified, usually because no
                                     trust information (e.g. a cryptographic signature) was provided */
        CompletelyUntrusted,        /**< The signature is broken/expired/false */
        UnknownTrusted,             /**< The signature is valid, but the key is unknown */
        UserTrusted,                /**< The signature is valid and made with a key signed by one of the
                                     user's own keys*/
        SelfTrusted,                /**< The signature is valid and made with one of the user's own keys*/
        FullyTrusted,               /**< The signature is valid and made with a key signed by the vendor's key*/
        UltimatelyTrusted           /**< The signature is valid and made with the vendor's key*/
    };
    Q_ENUM(TrustLevel)

    /**
     * Description on how draw a background for the applet
     */
    enum BackgroundHints {
        NoBackground = 0,         /**< Not drawing a background under the applet, the applet has its own implementation */
        StandardBackground = 1,   /**< The standard background from the theme is drawn */
        TranslucentBackground = 2, /**< An alternate version of the background is drawn, usually more translucent */
        DefaultBackground = StandardBackground /**< Default settings: both standard background */
    };
    Q_ENUM(BackgroundHints)

private:
    Types(QObject *parent = 0);
};

/**
 * Converts a location to a direction. Handy for figuring out which way to send a popup based on
 * location or to point arrows and other directional items.
 *
 * @param location the location of the container the element will appear in
 * @return the visual direction the element should be oriented in
 **/
PLASMA_EXPORT Types::Direction locationToDirection(Types::Location location);

/**
 * Converts a location to the direction facing it. Handy for figuring out which way to collapse
 * a popup or to point arrows at the item itself.
 *
 * @param location the location of the container the element will appear in
 * @return the visual direction the element should be oriented in
 **/
PLASMA_EXPORT Types::Direction locationToInverseDirection(Types::Location location);

} // Plasma namespace

Q_DECLARE_OPERATORS_FOR_FLAGS(Plasma::Types::Constraints)
Q_DECLARE_OPERATORS_FOR_FLAGS(Plasma::Types::Flip)
Q_DECLARE_OPERATORS_FOR_FLAGS(Plasma::Types::ComponentTypes)

#endif // multiple inclusion guard
