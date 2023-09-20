/*
    SPDX-FileCopyrightText: 2005 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
 * @class Types plasma/plasma.h <Plasma/Plasma>
 * @short Enums and constants used in Plasma
 *
 */
class PLASMA_EXPORT Types : public QObject
{
    Q_OBJECT

public:
    ~Types() override;

    /**
     * This enumeration lists the various hints that an applet can pass to its
     * constraint regarding the way that it is represented
     */
    enum ConstraintHints { // TODO KF6 ConstraintHints -> ConstraintHint
        NoHint = 0,
        CanFillArea = 1,
        /**< The CompactRepresentation can fill the area and ignore constraint margins*/ // (TODO: KF6 CanFillArea -> CompactRepresentationFillArea)
        MarginAreasSeparator = CanFillArea | 2, /**< The applet acts as a separator between the standard and slim panel margin areas*/
    };
    Q_DECLARE_FLAGS(ConstraintFlags, ConstraintHints)
    Q_FLAG(ConstraintFlags)

    /**
     * The FormFactor enumeration describes how a Plasma::Applet should arrange
     * itself. The value is derived from the container managing the Applet
     * (e.g. in Plasma, a Corona on the desktop or on a panel).
     **/
    enum FormFactor {
        Planar = 0, /**< The applet lives in a plane and has two
                   degrees of freedom to grow. Optimize for
                   desktop, laptop or tablet usage: a high
                   resolution screen 1-3 feet distant from the
                   viewer. */
        MediaCenter, /**< As with Planar, the applet lives in a plane
                    but the interface should be optimized for
                    medium-to-high resolution screens that are
                    5-15 feet distant from the viewer. Sometimes
                    referred to as a "ten foot interface".*/
        Horizontal, /**< The applet is constrained vertically, but
                   can expand horizontally. */
        Vertical, /**< The applet is constrained horizontally, but
                can expand vertically. */
        Application, /**< The Applet lives in a plane and should be optimized to look as a full application,
                     for the desktop or the particular device. */
    };
    Q_ENUM(FormFactor)

    /**
     * Display hints that come from the containment that suggest the applet how to look and behave.
     * @since 5.77
     */
    enum ContainmentDisplayHint {
        NoContainmentDisplayHint = 0,
        ContainmentDrawsPlasmoidHeading =
            1, /**< The containment will draw an titlebar-looking header for the applets, so the applets shouldn't attempt to paint a similar thing **/
        ContainmentForcesSquarePlasmoids =
            2, /**< The containment will force every plasmoid to be constrained in a square icon (An example is the System Tray)**/
        DesktopFullyCovered =
            4, /**< The desktop area for the contaiment's screen is not visible at all, for instance a window has been maximized on top of it */
    };
    Q_ENUM(ContainmentDisplayHint)
    Q_DECLARE_FLAGS(ContainmentDisplayHints, ContainmentDisplayHint)
    Q_FLAG(ContainmentDisplayHints)

    /**
     * The Location enumeration describes where on screen an element, such as an
     * Applet or its managing container, is positioned on the screen.
     **/
    enum Location {
        Floating = 0, /**< Free floating. Neither geometry or z-ordering
                     is described precisely by this value. */
        Desktop, /**< On the planar desktop layer, extending across
                the full screen from edge to edge */
        FullScreen, /**< Full screen */
        TopEdge, /**< Along the top of the screen*/
        BottomEdge, /**< Along the bottom of the screen*/
        LeftEdge, /**< Along the left side of the screen */
        RightEdge, /**< Along the right side of the screen */
    };
    Q_ENUM(Location)

    /**
     * Defines the immutability of items like applets, corona and containments
     * they can be free to modify, locked down by the user or locked down by the
     * system (e.g. kiosk setups).
     */
    enum ImmutabilityType {
        Mutable = 1, /**< The item can be modified in any way **/
        UserImmutable = 2, /**< The user has requested a lock down, and can undo
                          the lock down at any time **/
        SystemImmutable = 4, /**<  the item is locked down by the system, the user
                           can't unlock it **/
    };
    Q_ENUM(ImmutabilityType)

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
        // FIXME KF6: this should be the smallest status
        HiddenStatus = 6, /**< The Item will be hidden totally  **/
    };
    Q_ENUM(ItemStatus)

    /**
     * Description on how draw a background for the applet
     */
    enum BackgroundHints {
        NoBackground = 0, /**< Not drawing a background under the applet, the applet has its own implementation */
        StandardBackground = 1, /**< The standard background from the theme is drawn */
        TranslucentBackground = 2, /**< An alternate version of the background is drawn, usually more translucent */
        ShadowBackground = 4, /**< The applet won't have a svg background but a drop shadow of its content done via a shader */
        ConfigurableBackground = 8, /** If the hint has this flag, the user is able to configure this background */
        DefaultBackground = StandardBackground, /**< Default settings: both standard background */
    };
    Q_ENUM(BackgroundHints)
    // TODO KF6: BackgroundHint and BackgroundHints
    Q_DECLARE_FLAGS(BackgroundFlags, BackgroundHints)

private:
    Types(QObject *parent = nullptr);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Types::ContainmentDisplayHints)
Q_DECLARE_OPERATORS_FOR_FLAGS(Types::ConstraintFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(Types::BackgroundFlags)

} // Plasma namespace

#endif // multiple inclusion guard
