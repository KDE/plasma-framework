/******************************************************************************
*   Copyright 2011 by Aaron Seigo <aseigo@kde.org>                            *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#include "themedwidgetinterface_p.h"

#include "theme.h"

namespace Plasma
{

PaletteHelper *PaletteHelper::s_paletteHelper = 0;

PaletteHelper::PaletteHelper()
    : QObject()
{
    generatePalettes();
    connect(Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(generatePalettes()));
    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), this, SLOT(generatePalettes()));
}

PaletteHelper *PaletteHelper::self()
{
    if (!s_paletteHelper) {
        s_paletteHelper = new PaletteHelper;
    }

    return s_paletteHelper;
}

void PaletteHelper::generatePalettes()
{
    Theme *theme = Theme::defaultTheme();

    QColor color = theme->color(Theme::TextColor);
    palette = qApp->palette();
    palette.setColor(QPalette::Normal, QPalette::WindowText, color);
    palette.setColor(QPalette::Inactive, QPalette::WindowText, color);

    palette.setColor(QPalette::Normal, QPalette::Link, theme->color(Theme::LinkColor));
    palette.setColor(QPalette::Normal, QPalette::LinkVisited, theme->color(Theme::VisitedLinkColor));

    qreal alpha = color.alphaF();
    color.setAlphaF(0.6);
    palette.setColor(QPalette::Disabled, QPalette::WindowText, color);
    color.setAlphaF(alpha);

    palette.setColor(QPalette::Normal, QPalette::Text, color);
    palette.setColor(QPalette::Inactive, QPalette::Text, color);

    const QColor buttonColor = Theme::defaultTheme()->color(Theme::ButtonTextColor);
    palette.setColor(QPalette::Normal, QPalette::ButtonText, buttonColor);
    palette.setColor(QPalette::Inactive, QPalette::ButtonText, buttonColor);

    //FIXME: hardcoded colors .. looks incorrect
    palette.setColor(QPalette::Normal, QPalette::Base, QColor(0,0,0,0));
    palette.setColor(QPalette::Inactive, QPalette::Base, QColor(0,0,0,0));

    buttonPalette = palette;
    buttonPalette.setColor(QPalette::Normal, QPalette::Text, buttonColor);
    buttonPalette.setColor(QPalette::Inactive, QPalette::Text, buttonColor);

    emit palettesUpdated();
}


} // namespace Plasma

#include "moc_themedwidgetinterface_p.cpp"

