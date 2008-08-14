/*
 *   Copyright © 2008 Fredrik Höglund <fredrik@kde.org>
 *   Copyright © 2008 Marco Martin <notmart@gmail.com> 
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

#include "style.h"

#include <QPainter>
#include <QStyleOptionComplex>

#include <KDebug>

#include <plasma/panelsvg.h>

namespace Plasma {

class StylePrivate {
public:
    StylePrivate()
        : scrollbar(0)
    {
    }

    ~StylePrivate()
    {
    }
    
    Plasma::PanelSvg *scrollbar;
};

Style::Style()
     : QCommonStyle(),
       d(new StylePrivate)
{
    d->scrollbar = new Plasma::PanelSvg(this);
    d->scrollbar->setImagePath("widgets/scrollbar");
    d->scrollbar->setCacheAllRenderedPanels(true);
}

Style::~Style()
{
    delete d;
}

void Style::drawComplexControl(ComplexControl control,
                               const QStyleOptionComplex *option,
                               QPainter *painter,
                               const QWidget *widget) const
{
    if (control != CC_ScrollBar) {
        QCommonStyle::drawComplexControl(control, option, painter, widget);
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    const QStyleOptionSlider *scrollOption = qstyleoption_cast<const QStyleOptionSlider *>(option);

    QRect subLine;
    QRect addLine;
    if (scrollOption && scrollOption->orientation == Qt::Horizontal) {
        subLine = d->scrollbar->elementRect("arrow-left").toRect();
        addLine = d->scrollbar->elementRect("arrow-right").toRect();
    } else {
        subLine = d->scrollbar->elementRect("arrow-up").toRect();
        addLine = d->scrollbar->elementRect("arrow-down").toRect();
    }
    
    subLine.moveCenter(subControlRect(control, option, SC_ScrollBarSubLine, widget).center());
    addLine.moveCenter(subControlRect(control, option, SC_ScrollBarAddLine, widget).center());
    
    const QRect slider = subControlRect(control, option, SC_ScrollBarSlider, widget).adjusted(1, 0, -1, 0);

    d->scrollbar->setElementPrefix("background");
    d->scrollbar->resizePanel(option->rect.size());
    d->scrollbar->paintPanel(painter);
    
    d->scrollbar->setElementPrefix("slider");
    d->scrollbar->resizePanel(slider.size());
    d->scrollbar->paintPanel(painter, slider.topLeft());

    if (scrollOption && scrollOption->orientation == Qt::Horizontal) {
        d->scrollbar->paint(painter, addLine.topLeft(), "arrow-left");
        d->scrollbar->paint(painter, subLine.topLeft(), "arrow-right");
    } else {
        d->scrollbar->paint(painter, addLine.topLeft(), "arrow-down");
        d->scrollbar->paint(painter, subLine.topLeft(), "arrow-up"); 
    }
    
    painter->restore();
}

}

