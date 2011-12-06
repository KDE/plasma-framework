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

class PaletteHelper : public QObject
{
    Q_OBJECT
public:
    static PaletteHelper *self();

public Q_SLOTS:
    void generatePalettes();

Q_SIGNALS:
    void palettesUpdated();

public:
    QPalette palette;
    QPalette buttonPalette;

private:
    PaletteHelper();
    static PaletteHelper *s_paletteHelper;
};


template <class T>
class ThemedWidgetInterface
{
public:
    ThemedWidgetInterface(T *publicClass)
        : q(publicClass),
          customPalette(false),
          customFont(false),
          buttonColorForText(false),
          internalPaletteChange(false)
    {
        QObject::connect(PaletteHelper::self(), SIGNAL(palettesUpdated()), q, SLOT(setPalette()));
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
            internalPaletteChange = true;
            q->setPalette((buttonColorForText ? PaletteHelper::self()->buttonPalette
                                              : PaletteHelper::self()->palette));
            internalPaletteChange = false;
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
                if (!internalPaletteChange &&
                    q->palette() != (buttonColorForText ?  PaletteHelper::self()->buttonPalette : PaletteHelper::self()->palette)) {
                    customPalette = true;
                }
                break;

            default:
                break;
        }
    }

    void setWidget(QWidget *widget)
    {
        internalPaletteChange = true;
        q->setWidget(widget);
        internalPaletteChange = false;
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
    bool buttonColorForText : 1;
    bool internalPaletteChange : 1;
};

} // namespace Plasma
#endif

