/******************************************************************************
*   Copyright 2009 by Aaron Seigo <aseigo@kde.org>                            *
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

#ifndef THEMEDWIDGETINTERFACE_P_H
#define THEMEDWIDGETINTERFACE_P_H

#include "kglobalsettings.h"
#include <QApplication>

#include "theme.h"

namespace Plasma
{

template <class T>
class ThemedWidgetInterface
{
public:
    ThemedWidgetInterface(T *publicClass)
        : q(publicClass),
          customPalette(false),
          customFont(false)
    {
        QObject::connect(Theme::defaultTheme(), SIGNAL(themeChanged()), q, SLOT(setPalette()));
        QObject::connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), q, SLOT(setPalette()));
    }

    void initTheming()
    {
        customPalette = false;
        customFont = false;
        setPalette();
    }

    void setPalette()
    {
        if (!customPalette) {
            QColor color = Theme::defaultTheme()->color(Theme::TextColor);
            QPalette p = q->palette();
            p.setColor(QPalette::Normal, QPalette::WindowText, color);
            p.setColor(QPalette::Inactive, QPalette::WindowText, color);
            p.setColor(QPalette::Normal, QPalette::Text, color);
            p.setColor(QPalette::Inactive, QPalette::Text, color);
            color.setAlphaF(0.6);
            p.setColor(QPalette::Disabled, QPalette::WindowText, color);

            p.setColor(QPalette::Normal, QPalette::Link, Theme::defaultTheme()->color(Theme::LinkColor));
            p.setColor(QPalette::Normal, QPalette::LinkVisited, Theme::defaultTheme()->color(Theme::VisitedLinkColor));

            p.setColor(QPalette::Normal, QPalette::ButtonText, color);
            p.setColor(QPalette::Inactive, QPalette::ButtonText, color);

            //FIXME: hardcoded colors .. looks incorrect
            p.setColor(QPalette::Normal, QPalette::Base, QColor(0,0,0,0));
            p.setColor(QPalette::Inactive, QPalette::Base, QColor(0,0,0,0));

            q->setPalette(p);
            customPalette = false;
        }

        if (!customFont) {
            q->setFont(Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont));
            customFont = false;
        }
    }

    void changeEvent(QEvent *event)
    {
        switch (event->type()) {
            case QEvent::FontChange:
                customFont = q->font() != QApplication::font();
                break;

            case QEvent::PaletteChange:
             //   customPalette = true;
                break;

            default:
                break;
        }
    }

    void event(QEvent *event)
    {
        if (event->type() == QEvent::Show) {
            customFont = q->font() != QApplication::font();
        }
    }

    T *q;
    bool customPalette : 1;
    bool customFont : 1;
};

} // namespace Plasma
#endif

