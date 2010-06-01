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
#include <QPaintEngine>
#include <QPixmap>

#include "private/effects/blur.cpp"
#include "private/effects/halopainter_p.h"

namespace Plasma
{

namespace PaintUtils
{

void shadowBlur(QImage &image, int radius, const QColor &color)
{
    if (radius < 1) {
        return;
    }
    if (image.isNull()) {
        return;
    }

    expblur<16, 7>(image, radius);

    QPainter p(&image);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(image.rect(), color);
    p.end();
}

//TODO: we should have shadowText methods that paint the results directly into a QPainter passed in
QPixmap shadowText(QString text, QColor textColor, QColor shadowColor, QPoint offset, int radius)
{
    return shadowText(text, qApp->font(), textColor, shadowColor, offset, radius);
}

QPixmap shadowText(QString text, const QFont &font, QColor textColor, QColor shadowColor, QPoint offset, int radius)
{
    //don't try to paint stuff on a future null pixmap because the text is empty
    if (text.isEmpty()) {
        return QPixmap();
    }

    // Draw text
    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(text);
    QPixmap textPixmap(textRect.width(), fm.height());
    textPixmap.fill(Qt::transparent);
    QPainter p(&textPixmap);
    p.setPen(textColor);
    p.setFont(font);
    // FIXME: the center alignment here is odd: the rect should be the size needed by
    //        the text, but for some fonts and configurations this is off by a pixel or so
    //        and "centering" the text painting 'fixes' that. Need to research why
    //        this is the case and determine if we should be painting it differently here,
    //        doing soething different with the boundingRect call or if it's a problem
    //        in Qt itself
    p.drawText(textPixmap.rect(), Qt::AlignCenter, text);
    p.end();

    //Draw blurred shadow
    QImage img(textRect.size() + QSize(radius * 2, radius * 2),
    QImage::Format_ARGB32_Premultiplied);
    img.fill(0);
    p.begin(&img);
    p.drawImage(QPoint(radius, radius), textPixmap.toImage());
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

void drawHalo(QPainter *painter, const QRectF &rect)
{
    HaloPainter::instance()->drawHalo(painter, rect.toRect());
}

QPainterPath roundedRectangle(const QRectF &rect, qreal radius)
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

void centerPixmaps(QPixmap &from, QPixmap &to)
{
    if (from.size() == to.size() && from.hasAlphaChannel() && to.hasAlphaChannel()) {
        return;
    }
    QRect fromRect(from.rect());
    QRect toRect(to.rect());
 
    QRect actualRect = QRect(QPoint(0,0), fromRect.size().expandedTo(toRect.size()));
    fromRect.moveCenter(actualRect.center());
    toRect.moveCenter(actualRect.center());

    if (from.size() != actualRect.size() || !from.hasAlphaChannel()) {
        QPixmap result(actualRect.size());
        result.fill(Qt::transparent);
        QPainter p(&result);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.drawPixmap(fromRect.topLeft(), from);
        p.end();
        from = result;
    }

    if (to.size() != actualRect.size() || !to.hasAlphaChannel()) {
        QPixmap result(actualRect.size());
        result.fill(Qt::transparent);
        QPainter p(&result);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.drawPixmap(toRect.topLeft(), to);
        p.end();
        to = result;
    }
}

QPixmap transition(const QPixmap &from, const QPixmap &to, qreal amount)
{
    if (from.isNull() && to.isNull()) {
        return from;
    }

    QPixmap startPixmap(from);
    QPixmap targetPixmap(to);

    if (from.size() != to.size() || !from.hasAlphaChannel() || !to.hasAlphaChannel()) {
        centerPixmaps(startPixmap, targetPixmap);
    }

    //paint to in the center of from
    QRect toRect = to.rect();

    QColor color;
    color.setAlphaF(amount);


    // If the native paint engine supports Porter/Duff compositing and CompositionMode_Plus
    QPaintEngine *paintEngine = from.paintEngine();
    if (paintEngine && 
        paintEngine->hasFeature(QPaintEngine::PorterDuff) &&
        paintEngine->hasFeature(QPaintEngine::BlendModes)) {

        QPainter p;
        p.begin(&targetPixmap);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(targetPixmap.rect(), color);
        p.end();

        p.begin(&startPixmap);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.fillRect(startPixmap.rect(), color);
        p.setCompositionMode(QPainter::CompositionMode_Plus);
        p.drawPixmap(toRect.topLeft(), targetPixmap);
        p.end();

        return startPixmap;
    }
#if defined(Q_WS_X11) && defined(HAVE_XRENDER)
    // We have Xrender support
    else if (paintEngine && paintEngine->hasFeature(QPaintEngine::PorterDuff)) {
        // QX11PaintEngine doesn't implement CompositionMode_Plus in Qt 4.3,
        // which we need to be able to do a transition from one pixmap to
        // another.
        //
        // In order to avoid the overhead of converting the pixmaps to images
        // and doing the operation entirely in software, this function has a
        // specialized path for X11 that uses Xrender directly to do the
        // transition. This operation can be fully accelerated in HW.
        //
        // This specialization can be removed when QX11PaintEngine supports
        // CompositionMode_Plus.
        QPixmap source(targetPixmap), destination(startPixmap);

        source.detach();
        destination.detach();

        Display *dpy = QX11Info::display();

        XRenderPictFormat *format = XRenderFindStandardFormat(dpy, PictStandardA8);
        XRenderPictureAttributes pa;
        pa.repeat = 1; // RepeatNormal

        // Create a 1x1 8 bit repeating alpha picture
        Pixmap pixmap = XCreatePixmap(dpy, destination.handle(), 1, 1, 8);
        Picture alpha = XRenderCreatePicture(dpy, pixmap, format, CPRepeat, &pa);
        XFreePixmap(dpy, pixmap);

        // Fill the alpha picture with the opacity value
        XRenderColor xcolor;
        xcolor.alpha = quint16(0xffff * amount);
        XRenderFillRectangle(dpy, PictOpSrc, alpha, &xcolor, 0, 0, 1, 1);

        // Reduce the alpha of the destination with 1 - opacity
        XRenderComposite(dpy, PictOpOutReverse, alpha, None, destination.x11PictureHandle(),
                         0, 0, 0, 0, 0, 0, destination.width(), destination.height());

        // Add source * opacity to the destination
        XRenderComposite(dpy, PictOpAdd, source.x11PictureHandle(), alpha,
                         destination.x11PictureHandle(),
                         toRect.x(), toRect.y(), 0, 0, 0, 0, destination.width(), destination.height());

        XRenderFreePicture(dpy, alpha);
        return destination;
    }
#endif
    else {
        // Fall back to using QRasterPaintEngine to do the transition.
        QImage under = startPixmap.toImage();
        QImage over  = targetPixmap.toImage();

        QPainter p;
        p.begin(&over);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(over.rect(), color);
        p.end();

        p.begin(&under);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.fillRect(under.rect(), color);
        p.setCompositionMode(QPainter::CompositionMode_Plus);
        p.drawImage(toRect.topLeft(), over);
        p.end();

        return QPixmap::fromImage(under);
    }
}

} // PaintUtils namespace

} // Plasma namespace

