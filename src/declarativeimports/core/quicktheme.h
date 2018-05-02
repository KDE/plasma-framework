/*
 *   Copyright 2006-2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2013 Marco Martin <mart@kde.org>
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

#ifndef PLASMA_QUICKTHEME_H
#define PLASMA_QUICKTHEME_H

#include <plasma/theme.h>

#include <QColor>

namespace Plasma
{

/**
 * @class QuickTheme plasma/theme.h <Plasma/Theme>
 *
 * @short Interface to the Plasma theme
 *
 *
 * Plasma::Theme provides access to a common and standardized set of graphic
 * elements stored in SVG format. This allows artists to create single packages
 * of SVGs that will affect the look and feel of all workspace components.
 *
 * Plasma::Svg uses Plasma::Theme internally to locate and load the appropriate
 * SVG data. Alternatively, Plasma::Theme can be used directly to retrieve
 * file system paths to SVGs by name.
 */
class QuickTheme : public Plasma::Theme
{
    Q_OBJECT

    // colors
    Q_PROPERTY(QColor textColor READ textColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor highlightColor READ highlightColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor highlightedTextColor READ highlightedTextColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor linkColor READ linkColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor visitedLinkColor READ visitedLinkColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor positiveTextColor READ positiveTextColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor neutralTextColor READ neutralTextColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor negativeTextColor READ negativeTextColor NOTIFY themeChangedProxy)

    Q_PROPERTY(QColor buttonTextColor READ buttonTextColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor buttonBackgroundColor READ buttonBackgroundColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor buttonHoverColor READ buttonHoverColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor buttonFocusColor READ buttonFocusColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor buttonHighlightedTextColor READ buttonHighlightedTextColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor buttonPositiveTextColor READ buttonPositiveTextColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor buttonNeutralTextColor READ buttonNeutralTextColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor buttonNegativeTextColor READ buttonNegativeTextColor NOTIFY themeChangedProxy)

    Q_PROPERTY(QColor viewTextColor READ viewTextColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor viewBackgroundColor READ viewBackgroundColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor viewHoverColor READ viewHoverColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor viewFocusColor READ viewFocusColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor viewHighlightedTextColor READ viewHighlightedTextColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor viewPositiveTextColor READ viewPositiveTextColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor viewNeutralTextColor READ viewNeutralTextColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor viewNegativeTextColor READ viewNegativeTextColor NOTIFY themeChangedProxy)

    Q_PROPERTY(QColor complementaryTextColor READ complementaryTextColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor complementaryBackgroundColor READ complementaryBackgroundColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor complementaryHoverColor READ complementaryHoverColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor complementaryFocusColor READ complementaryFocusColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor complementaryHighlightedTextColor READ complementaryHighlightedTextColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor complementaryPositiveTextColor READ buttonPositiveTextColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor complementaryNeutralTextColor READ buttonNeutralTextColor NOTIFY themeChangedProxy)
    Q_PROPERTY(QColor complementaryNegativeTextColor READ buttonNegativeTextColor NOTIFY themeChangedProxy)

public:
    explicit QuickTheme(QObject *parent = nullptr);
    ~QuickTheme();



    /**
     * @return The theme's colorscheme's text color
     * @since 5.0
     */
    QColor textColor() const;

    /**
     * @return The theme's colorscheme's highlight color
     * @since 5.0
     */
    QColor highlightColor() const;

    /**
     * @return The theme's colorscheme's highlighted text color
     * @since 5.22
     */
    QColor highlightedTextColor() const;

    /**
     * @return The theme's colorscheme's positive text color
     * @since 5.22
     */
    QColor positiveTextColor() const;

    /**
     * @return The theme's colorscheme's neutral text color
     * @since 5.22
     */
    QColor neutralTextColor() const;

    /**
     * @return The theme's colorscheme's negative text color
     * @since 5.22
     */
    QColor negativeTextColor() const;

    /**
     * @return The theme's colorscheme's background color
     * @since 5.0
     */
    QColor backgroundColor() const;

    /**
     * @return The theme's colorscheme's color for text on buttons
     * @since 5.0
     */
    QColor buttonTextColor() const;

    /**
     * @return The theme's colorscheme's background color color of buttons
     * @since 5.0
     */
    QColor buttonBackgroundColor() const;

    /**
     * @return The theme's colorscheme's positive text color of buttons
     * @since 5.22
     */
    QColor buttonPositiveTextColor() const;

    /**
     * @return The theme's colorscheme's neutral text color of buttons
     * @since 5.22
     */
    QColor buttonNeutralTextColor() const;

    /**
     * @return The theme's colorscheme's negative text color of buttons
     * @since 5.22
     */
    QColor buttonNegativeTextColor() const;

    /**
     * @return The theme's colorscheme's link color
     * @since 5.0
     */
    QColor linkColor() const;

    /**
     * @return The theme's colorscheme's text color for visited links
     * @since 5.0
     */
    QColor visitedLinkColor() const;

    /**
     * @return The theme's colorscheme's color of hovered buttons
     * @since 5.0
     */
    QColor buttonHoverColor() const;

    /**
     * @return The theme's colorscheme's color of focused buttons
     * @since 5.0
     */
    QColor buttonFocusColor() const;

    /**
     * @return The theme's colorscheme's highlighted text color for buttons
     * @since 5.22
     */
    QColor buttonHighlightedTextColor() const;

    /**
     * @return The theme's colorscheme's text color in views
     * @since 5.0
     */
    QColor viewTextColor() const;

    /**
     * @return The theme's colorscheme's background color of views
     * @since 5.0
     */
    QColor viewBackgroundColor() const;

    /**
     * @return The theme's colorscheme's color of hovered views
     * @since 5.0
     */
    QColor viewHoverColor() const;

    /**
     * @return The theme's colorscheme's color of focused views
     * @since 5.0
     */
    QColor viewFocusColor() const;

    /**
     * @return The theme's colorscheme's highlighted text color for views
     * @since 5.22
     */
    QColor viewHighlightedTextColor() const;

    /**
     * @return The theme's colorscheme's positive text color of view
     * @since 5.22
     */
    QColor viewPositiveTextColor() const;

    /**
     * @return The theme's colorscheme's neutral text color of view
     * @since 5.22
     */
    QColor viewNeutralTextColor() const;

    /**
     * @return The theme's colorscheme's negative text color of view
     * @since 5.22
     */
    QColor viewNegativeTextColor() const;

    /**
     * @return The theme's colorscheme's text color of "complementary" areas
     * @since 5.0
     */
    QColor complementaryTextColor() const;

    /**
     * @return The theme's colorscheme's background color of "complementary" areas
     * @since 5.0
     */
    QColor complementaryBackgroundColor() const;

    /**
     * @return The theme's colorscheme's color of hovered "complementary" areas
     * @since 5.0
     */
    QColor complementaryHoverColor() const;

    /**
     * @return The theme's colorscheme's color of focused "complementary" areas
     * @since 5.0
     */
    QColor complementaryFocusColor() const;

    /**
     * @return The theme's colorscheme's highlighted text color for "complementary" areas
     * @since 5.22
     */
    QColor complementaryHighlightedTextColor() const;

    /**
     * @return The theme's colorscheme's positive text color of complementary
     * @since 5.22
     */
    QColor complementaryPositiveTextColor() const;

    /**
     * @return The theme's colorscheme's neutral text color of complementary
     * @since 5.22
     */
    QColor complementaryNeutralTextColor() const;

    /**
     * @return The theme's colorscheme's negative text color of complementary
     * @since 5.22
     */
    QColor complementaryNegativeTextColor() const;

Q_SIGNALS:
    void themeChangedProxy();
};

} // Plasma namespace

#endif // multiple inclusion guard

