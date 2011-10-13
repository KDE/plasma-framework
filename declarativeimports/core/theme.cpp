/***************************************************************************
 *   Copyright 2010 Marco Martin <mart@kde.org>                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "theme_p.h"

#include <plasma/theme.h>

class FontProxySingleton
{
public:
   FontProxy self;
};

K_GLOBAL_STATIC(FontProxySingleton, privateFontProxySingleton)

FontProxy::FontProxy(QObject *parent)
    : QObject(parent)
{
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(boldChanged()));
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(capitalizationChanged()));
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(familyChanged()));
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(italicChanged()));
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(letterSpacingChanged()));
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(pixelSizeChanged()));
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(pointSizeChanged()));
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(strikeOutChanged()));
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(underlineChanged()));
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(weightChanged()));
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(wordSpacingChanged()));
}

FontProxy::~FontProxy()
{
}

FontProxy *FontProxy::self()
{
    return &privateFontProxySingleton->self;
}

bool FontProxy::bold() const
{
    return Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont).bold();
}

FontProxy::Capitalization FontProxy::capitalization() const
{
    return (FontProxy::Capitalization)Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont).capitalization();
}

QString FontProxy::family() const
{
    return Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont).family();
}

bool FontProxy::italic() const
{
    return Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont).italic();
}

qreal FontProxy::letterSpacing() const
{
    return Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont).letterSpacing();
}

int FontProxy::pixelSize() const
{
    return Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont).pixelSize();
}

qreal FontProxy::pointSize() const
{
    return Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont).pointSize();
}

bool FontProxy::strikeOut() const
{
    return Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont).strikeOut();
}

bool FontProxy::underline() const
{
    return Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont).underline();
}

FontProxy::Weight FontProxy::weight() const
{
    return (FontProxy::Weight)Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont).weight();
}

qreal FontProxy::wordSpacing() const
{
    return Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont).wordSpacing();
}



//********** Theme *************

ThemeProxy::ThemeProxy(QObject *parent)
    : QObject(parent)
{
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SIGNAL(themeChanged()));
}

ThemeProxy::~ThemeProxy()
{
}

QString ThemeProxy::themeName() const
{
    return Plasma::Theme::defaultTheme()->themeName();
}

QObject *ThemeProxy::font() const
{
    return FontProxy::self();
}

bool ThemeProxy::windowTranslucencyEnabled() const
{
    return Plasma::Theme::defaultTheme()->windowTranslucencyEnabled();
}

KUrl ThemeProxy::homepage() const
{
    return Plasma::Theme::defaultTheme()->homepage();
}

bool ThemeProxy::useGlobalSettings() const
{
    return Plasma::Theme::defaultTheme()->useGlobalSettings();
}

QString ThemeProxy::wallpaperPath() const
{
    return Plasma::Theme::defaultTheme()->wallpaperPath();
}

QString ThemeProxy::wallpaperPathForSize(int width, int height) const
{
    return Plasma::Theme::defaultTheme()->wallpaperPath(QSize(width, height));
}

QColor ThemeProxy::textColor() const
{
    return Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
}

QColor ThemeProxy::highlightColor() const
{
    return Plasma::Theme::defaultTheme()->color(Plasma::Theme::HighlightColor);
}

QColor ThemeProxy::backgroundColor() const
{
    return Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
}

QColor ThemeProxy::buttonTextColor() const
{
    return Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonTextColor);
}

QColor ThemeProxy::buttonBackgroundColor() const
{
    return Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonBackgroundColor);
}

QColor ThemeProxy::linkColor() const
{
    return Plasma::Theme::defaultTheme()->color(Plasma::Theme::LinkColor);
}

QColor ThemeProxy::visitedLinkColor() const
{
    return Plasma::Theme::defaultTheme()->color(Plasma::Theme::VisitedLinkColor);
}

QColor ThemeProxy::buttonHoverColor() const
{
    return Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonHoverColor);
}

QColor ThemeProxy::buttonFocusColor() const
{
    return Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonFocusColor);
}

QColor ThemeProxy::viewTextColor() const
{
    return Plasma::Theme::defaultTheme()->color(Plasma::Theme::ViewTextColor);
}

QColor ThemeProxy::viewBackgroundColor() const
{
    return Plasma::Theme::defaultTheme()->color(Plasma::Theme::ViewBackgroundColor);
}

QColor ThemeProxy::viewHoverColor() const
{
    return Plasma::Theme::defaultTheme()->color(Plasma::Theme::ViewHoverColor);
}

QColor ThemeProxy::viewFocusColor() const
{
    return Plasma::Theme::defaultTheme()->color(Plasma::Theme::ViewFocusColor);
}

QString ThemeProxy::styleSheet() const
{
    return Plasma::Theme::defaultTheme()->styleSheet(QString());
}


#include "theme_p.moc"

