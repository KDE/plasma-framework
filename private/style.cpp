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

#include "style_p.h"

#include <QPainter>
#include <QStyleOptionComplex>
#include <QSpinBox>
#include <QComboBox>
#include <QApplication>

#include <kdebug.h>

#include <plasma/framesvg.h>
#include <plasma/theme.h>

namespace Plasma {

class StylePrivate
{
public:
    StylePrivate(Style *style)
        : q(style),
          scrollbar(0),
          textBox(0)
    {
    }

    ~StylePrivate()
    {
    }

    void createScrollbar()
    {
        if (!scrollbar) {
            scrollbar = new Plasma::FrameSvg(q);
            scrollbar->setImagePath("widgets/scrollbar");
            scrollbar->setCacheAllRenderedFrames(true);
        }
    }

    void createTextBox()
    {
        if (!textBox) {
            textBox = new Plasma::FrameSvg(q);
            textBox->setImagePath("widgets/frame");
            textBox->setElementPrefix("sunken");
        }
    }

    Style *q;
    Plasma::FrameSvg *scrollbar;
    Plasma::FrameSvg *textBox;
    static Plasma::Style::Ptr s_sharedStyle;
};

Style::Ptr StylePrivate::s_sharedStyle(0);

Style::Ptr Style::sharedStyle()
{
    if (!StylePrivate::s_sharedStyle) {
        StylePrivate::s_sharedStyle = new Style();
    }

    return StylePrivate::s_sharedStyle;
}

void Style::doneWithSharedStyle()
{
    if (StylePrivate::s_sharedStyle.isUnique()) {
        StylePrivate::s_sharedStyle = 0;
    }
}

Style::Style()
     : QCommonStyle(),
       d(new StylePrivate(this))
{
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
    if (Theme::defaultTheme()->useNativeWidgetStyle()) {
        qApp->style()->drawComplexControl(control, option, painter, widget);
        return;
    }

    switch (control) {
    case CC_ScrollBar: {
        d->createScrollbar();

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        const bool sunken = option->state & State_Sunken;
        const QStyleOptionSlider *scrollOption = qstyleoption_cast<const QStyleOptionSlider *>(option);
        QString prefix;

        if (option->state & State_MouseOver) {
            prefix= "mouseover-";
        }

        QRect subLine;
        QRect addLine;
        if (scrollOption && scrollOption->orientation == Qt::Horizontal) {
            subLine = d->scrollbar->elementRect(prefix + "arrow-left").toRect();
            addLine = d->scrollbar->elementRect(prefix + "arrow-right").toRect();
        } else {
            subLine = d->scrollbar->elementRect(prefix + "arrow-up").toRect();
            addLine = d->scrollbar->elementRect(prefix + "arrow-down").toRect();
        }

        subLine.moveCenter(subControlRect(control, option, SC_ScrollBarSubLine, widget).center());
        addLine.moveCenter(subControlRect(control, option, SC_ScrollBarAddLine, widget).center());

        const QRect slider =
            subControlRect(control, option, SC_ScrollBarSlider, widget).adjusted(1, 0, -1, 0);

        d->scrollbar->setElementPrefix("background");
        d->scrollbar->resizeFrame(option->rect.size());
        d->scrollbar->paintFrame(painter);

        if (sunken && scrollOption && scrollOption->activeSubControls & SC_ScrollBarSlider) {
            d->scrollbar->setElementPrefix("sunken-slider");
        } else {
            d->scrollbar->setElementPrefix(prefix + "slider");
        }

        d->scrollbar->resizeFrame(slider.size());
        d->scrollbar->paintFrame(painter, slider.topLeft());

        if (scrollOption && scrollOption->orientation == Qt::Horizontal) {
            if (sunken && scrollOption->activeSubControls & SC_ScrollBarAddLine) {
                d->scrollbar->paint(painter, addLine, "sunken-arrow-right");
            } else {
                d->scrollbar->paint(painter, addLine, prefix + "arrow-right");
            }

            if (sunken && scrollOption->activeSubControls & SC_ScrollBarSubLine) {
                d->scrollbar->paint(painter, subLine, "sunken-arrow-left");
            } else {
                d->scrollbar->paint(painter, subLine, prefix + "arrow-left");
            }
        } else {
            if (sunken && scrollOption && scrollOption->activeSubControls & SC_ScrollBarAddLine) {
                d->scrollbar->paint(painter, addLine, "sunken-arrow-down");
            } else {
                d->scrollbar->paint(painter, addLine, prefix + "arrow-down");
            }

            if (sunken && scrollOption && scrollOption->activeSubControls & SC_ScrollBarSubLine) {
                d->scrollbar->paint(painter, subLine, "sunken-arrow-up");
            } else {
                d->scrollbar->paint(painter, subLine, prefix + "arrow-up");
            }
        }

        painter->restore();
        break;
    }
    case CC_SpinBox: {
        d->createTextBox();

        d->textBox->resizeFrame(option->rect.size());
        d->textBox->paintFrame(painter);

        const QStyleOptionSpinBox *spinOpt = qstyleoption_cast<const QStyleOptionSpinBox *>(option);
        bool upSunken = (spinOpt->activeSubControls & SC_SpinBoxUp) &&
                         (spinOpt->state & (State_Sunken | State_On));
        bool downSunken = (spinOpt->activeSubControls & SC_SpinBoxDown) &&
                           (spinOpt->state & (State_Sunken | State_On));

        const QSpinBox *spin = qobject_cast<const QSpinBox *>(widget);
        PrimitiveElement pe;
        if (spin->buttonSymbols() == QSpinBox::PlusMinus) {
            pe = PE_IndicatorSpinPlus;
        } else {
            pe = PE_IndicatorArrowUp;
        }

        QStyleOption upOpt;
        upOpt = *option;
        upOpt.rect = subControlRect(CC_SpinBox, option, SC_SpinBoxUp, widget);

        if (upSunken) {
            upOpt.state = State_Sunken|State_Enabled;
        } else {
            upOpt.state = State_Enabled;
        }

        qApp->style()->drawPrimitive(pe, &upOpt, painter, widget);

        if (spin->buttonSymbols() == QSpinBox::PlusMinus) {
            pe = PE_IndicatorSpinMinus;
        } else {
            pe = PE_IndicatorArrowDown;
        }

        QStyleOption downOpt;
        downOpt= *option;
        downOpt.rect = subControlRect(CC_SpinBox, option, SC_SpinBoxDown, widget);

        if (downSunken) {
            downOpt.state = State_Sunken|State_Enabled;
        } else {
            downOpt.state = State_Enabled;
        }

        qApp->style()->drawPrimitive(pe, &downOpt, painter, widget);
        break;
    }
    case CC_ComboBox: {
        const QComboBox *combo = qobject_cast<const QComboBox *>(widget);
        if (!combo->isEditable()) {
            qApp->style()->drawComplexControl(control, option, painter, widget);
        } else {
            d->createTextBox();
            d->textBox->resizeFrame(option->rect.size());
            d->textBox->paintFrame(painter);

            QStyleOption arrowOpt;
            arrowOpt = *option;
            arrowOpt.rect = subControlRect(CC_ComboBox, option, SC_ComboBoxArrow, widget);
            qApp->style()->drawPrimitive(PE_IndicatorArrowDown, &arrowOpt, painter, widget);
        }
        break;
    }
    default:
        qApp->style()->drawComplexControl(control, option, painter, widget);
    }
}

void Style::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (Theme::defaultTheme()->useNativeWidgetStyle()) {
        qApp->style()->drawPrimitive(element, option, painter, widget);
        return;
    }

    switch (element) {
    case PE_PanelLineEdit:
        //comboboxes draws their own frame
        if (qobject_cast<QComboBox *>(widget->parent())) {
            return;
        }
        d->createTextBox();

        d->textBox->resizeFrame(option->rect.size());
        d->textBox->paintFrame(painter);
        break;
    default:
        qApp->style()->drawPrimitive(element, option, painter, widget);
    }
}

int Style::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    if (Theme::defaultTheme()->useNativeWidgetStyle()) {
        return qApp->style()->pixelMetric(metric, option, widget);
    }

    switch (metric) {
    case PM_ScrollBarExtent: {
        d->createScrollbar();
        const QStyleOptionSlider *scrollOption = qstyleoption_cast<const QStyleOptionSlider *>(option);
        if (scrollOption && scrollOption->orientation == Qt::Vertical) {
            return d->scrollbar->elementSize("arrow-down").width() + 2;
        } else {
            return d->scrollbar->elementSize("arrow-left").height() + 2;
        }
    }
    default:
        return QCommonStyle::pixelMetric(metric, option, widget);
    }
}

}

#include "style_p.moc"


