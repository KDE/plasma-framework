/*
    SPDX-FileCopyrightText: 2006-2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

QColor QuickTheme::disabledTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::DisabledTextColor);
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

QColor QuickTheme::headerTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::TextColor, Plasma::Theme::HeaderColorGroup);
}

QColor QuickTheme::headerBackgroundColor() const
{
    return Plasma::Theme::color(Plasma::Theme::BackgroundColor, Plasma::Theme::HeaderColorGroup);
}

QColor QuickTheme::headerHoverColor() const
{
    return Plasma::Theme::color(Plasma::Theme::HoverColor, Plasma::Theme::HeaderColorGroup);
}

QColor QuickTheme::headerFocusColor() const
{
    return Plasma::Theme::color(Plasma::Theme::FocusColor, Plasma::Theme::HeaderColorGroup);
}

QColor QuickTheme::headerHighlightedTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::HighlightedTextColor, Plasma::Theme::HeaderColorGroup);
}

QColor QuickTheme::headerPositiveTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::TextColor, Plasma::Theme::HeaderColorGroup);
}

QColor QuickTheme::headerNeutralTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::TextColor, Plasma::Theme::HeaderColorGroup);
}

QColor QuickTheme::headerNegativeTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::TextColor, Plasma::Theme::HeaderColorGroup);
}
}

#include "moc_quicktheme.cpp"
