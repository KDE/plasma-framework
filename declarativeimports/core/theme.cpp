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

QFont ThemeProxy::font() const
{
    return Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
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

