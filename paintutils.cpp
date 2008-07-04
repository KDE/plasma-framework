/*
 *   Copyright 2005 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Andrew Lake <jamboarder@yahoo.com>
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

#include <paintutils.h>

#include <QImage>
#include <QPainter>
#include <QPixmap>

#include "effects/blur.cpp"

namespace Plasma
{

namespace PaintUtils
{

void shadowBlur(QImage &image, int radius, const QColor &color)
{
    if (radius < 1)
        return;

    expblur<16, 7>(image, radius);

    QPainter p(&image);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(image.rect(), color);
}

QPixmap shadowText(QString text, QColor textColor, QColor shadowColor, QPoint offset, int radius)
{
    // Draw text 
    QFontMetrics fm(text);
    QRect textRect = fm.boundingRect(text);
    QPixmap textPixmap(textRect.size());
    textPixmap.fill(Qt::transparent);
    QPainter p(&textPixmap);
    p.setPen(textColor);
    p.drawText(textPixmap.rect(), Qt::AlignLeft, text);
    p.end();

    //Draw blurred shadow
    QImage img(textRect.size() + QSize(radius * 2, radius * 2),
    QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    p.begin(&img);
    p.drawImage(QPoint(radius,radius), textPixmap.toImage());
    p.end();
    shadowBlur(img, radius, shadowColor);

    //Compose text and shadow
    int addSizeX;
    int addSizeY;
    if (offset.x() > radius) {
        addSizeX = abs(offset.x()) - radius;
    } else {
        addSizeX = 0;
    }
    if (offset.y() > radius) {
        addSizeY = abs(offset.y()) - radius;
    } else {
        addSizeY = 0;
    }

    QPixmap finalPixmap(img.size() + QSize(addSizeX, addSizeY));
    finalPixmap.fill(Qt::transparent);
    p.begin(&finalPixmap);
    QPointF offsetF(offset);
    QPointF textTopLeft(finalPixmap.rect().topLeft() + 
            QPointF ((finalPixmap.width() - textPixmap.width()) / 2.0, (finalPixmap.height() - textPixmap.height()) / 2.0) -
            (offsetF / 2.0));     
    QPointF shadowTopLeft(finalPixmap.rect().topLeft() + 
            QPointF ((finalPixmap.width() - img.width()) / 2.0, (finalPixmap.height() - img.height()) / 2.0) + 
            (offsetF / 2.0));     

    p.drawImage(shadowTopLeft, img);
    p.drawPixmap(textTopLeft, textPixmap);
    p.end();

    return finalPixmap;
}

QPainterPath roundedRectangle(const QRectF& rect, qreal radius)
{
    QPainterPath path(QPointF(rect.left(), rect.top() + radius));
    path.quadTo(rect.left(), rect.top(), rect.left() + radius, rect.top());         // Top left corner
    path.lineTo(rect.right() - radius, rect.top());                                 // Top side
    path.quadTo(rect.right(), rect.top(), rect.right(), rect.top() + radius);       // Top right corner
    path.lineTo(rect.right(), rect.bottom() - radius);                              // Right side
    path.quadTo(rect.right(), rect.bottom(), rect.right() - radius, rect.bottom()); // Bottom right corner
    path.lineTo(rect.left() + radius, rect.bottom());                               // Bottom side
    path.quadTo(rect.left(), rect.bottom(), rect.left(), rect.bottom() - radius);   // Bottom left corner
    path.closeSubpath();

    return path;
}

} // PaintUtils namespace

} // Plasma namespace

