/*
 *   Copyright 2006-2007 Aaron Seigo <aseigo@kde.org>
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

#include "quicktheme.h"



namespace Plasma
{

QuickTheme::QuickTheme(QObject *parent)
    : Theme(parent)
{
    connect(this, &Theme::themeChanged, this, &QuickTheme::themeChangedProxy);
}

QuickTheme::~QuickTheme()
{
}

QColor QuickTheme::textColor() const
{
    return Plasma::Theme::color(Plasma::Theme::TextColor);
}

QColor QuickTheme::highlightColor() const
{
    return Plasma::Theme::color(Plasma::Theme::HighlightColor);
}

QColor QuickTheme::highlightedTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::HighlightedTextColor);
}

QColor QuickTheme::positiveTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::PositiveTextColor);
}

QColor QuickTheme::neutralTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::NeutralTextColor);
}

QColor QuickTheme::negativeTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::NegativeTextColor);
}

QColor QuickTheme::backgroundColor() const
{
    return Plasma::Theme::color(Plasma::Theme::BackgroundColor);
}

QColor QuickTheme::buttonTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::TextColor, Plasma::Theme::ButtonColorGroup);
}

QColor QuickTheme::buttonBackgroundColor() const
{
    return Plasma::Theme::color(Plasma::Theme::BackgroundColor, Plasma::Theme::ButtonColorGroup);
}

QColor QuickTheme::buttonPositiveTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::TextColor, Plasma::Theme::ButtonColorGroup);
}

QColor QuickTheme::buttonNeutralTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::TextColor, Plasma::Theme::ButtonColorGroup);
}

QColor QuickTheme::buttonNegativeTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::TextColor, Plasma::Theme::ButtonColorGroup);
}

QColor QuickTheme::linkColor() const
{
    return Plasma::Theme::color(Plasma::Theme::LinkColor);
}

QColor QuickTheme::visitedLinkColor() const
{
    return Plasma::Theme::color(Plasma::Theme::VisitedLinkColor);
}

QColor QuickTheme::buttonHoverColor() const
{
    return Plasma::Theme::color(Plasma::Theme::HoverColor, Plasma::Theme::ButtonColorGroup);
}

QColor QuickTheme::buttonFocusColor() const
{
    return Plasma::Theme::color(Plasma::Theme::FocusColor, Plasma::Theme::ButtonColorGroup);
}

QColor QuickTheme::buttonHighlightedTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::HighlightedTextColor, Plasma::Theme::ButtonColorGroup);
}

QColor QuickTheme::viewTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::TextColor, Plasma::Theme::ViewColorGroup);
}

QColor QuickTheme::viewBackgroundColor() const
{
    return Plasma::Theme::color(Plasma::Theme::BackgroundColor, Plasma::Theme::ViewColorGroup);
}

QColor QuickTheme::viewHoverColor() const
{
    return Plasma::Theme::color(Plasma::Theme::HoverColor, Plasma::Theme::ViewColorGroup);
}

QColor QuickTheme::viewFocusColor() const
{
    return Plasma::Theme::color(Plasma::Theme::FocusColor, Plasma::Theme::ViewColorGroup);
}

QColor QuickTheme::viewHighlightedTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::HighlightedTextColor, Plasma::Theme::ViewColorGroup);
}

QColor QuickTheme::viewPositiveTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::TextColor, Plasma::Theme::ViewColorGroup);
}

QColor QuickTheme::viewNeutralTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::TextColor, Plasma::Theme::ViewColorGroup);
}

QColor QuickTheme::viewNegativeTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::TextColor, Plasma::Theme::ViewColorGroup);
}

QColor QuickTheme::complementaryTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::TextColor, Plasma::Theme::ComplementaryColorGroup);
}

QColor QuickTheme::complementaryBackgroundColor() const
{
    return Plasma::Theme::color(Plasma::Theme::BackgroundColor, Plasma::Theme::ComplementaryColorGroup);
}

QColor QuickTheme::complementaryHoverColor() const
{
    return Plasma::Theme::color(Plasma::Theme::HoverColor, Plasma::Theme::ComplementaryColorGroup);
}

QColor QuickTheme::complementaryFocusColor() const
{
    return Plasma::Theme::color(Plasma::Theme::FocusColor, Plasma::Theme::ComplementaryColorGroup);
}

QColor QuickTheme::complementaryHighlightedTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::HighlightedTextColor, Plasma::Theme::ComplementaryColorGroup);
}

QColor QuickTheme::complementaryPositiveTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::TextColor, Plasma::Theme::ComplementaryColorGroup);
}

QColor QuickTheme::complementaryNeutralTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::TextColor, Plasma::Theme::ComplementaryColorGroup);
}

QColor QuickTheme::complementaryNegativeTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::TextColor, Plasma::Theme::ComplementaryColorGroup);
}

}

#include "moc_quicktheme.cpp"
