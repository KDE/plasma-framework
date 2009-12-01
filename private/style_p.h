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

#ifndef PLASMA_STYLE_P_H
#define PLASMA_STYLE_P_H

#include <QtCore/QSharedData>
#include <QtGui/QCommonStyle>

#include <ksharedptr.h>

namespace Plasma
{

class StylePrivate;

class Style : public QCommonStyle, public QSharedData
{
    Q_OBJECT

public:
    typedef KSharedPtr<Style> Ptr;

    static Style::Ptr sharedStyle();
    static void doneWithSharedStyle();

    explicit Style();
    ~Style();

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;

protected:
    void drawComplexControl(ComplexControl control,
                            const QStyleOptionComplex *option,
                            QPainter *painter,
                            const QWidget *widget) const;

    int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const;
    QRect subControlRect(ComplexControl control, const QStyleOptionComplex *option,
                         SubControl subControl, const QWidget *widget) const;

    QRect subElementRect(SubElement element, const QStyleOption * option, const QWidget *widget = 0) const;
    QSize sizeFromContents(ContentsType, const QStyleOption *option, const QSize &contentsSize,
                           const QWidget *widget = 0) const;
private:
    StylePrivate *d;
};

} // namespace Plasma

#endif // multiple inclusion guard
