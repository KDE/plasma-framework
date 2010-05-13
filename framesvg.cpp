/*
 *   Copyright 2008 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 Marco Martin <notmart@gmail.com>
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

#include "framesvg.h"
#include "private/framesvg_p.h"

#include <QPainter>
#include <QSize>
#include <QBitmap>
#include <QRegion>
#include <QTimer>
#include <QCryptographicHash>

#include <kdebug.h>

#include <plasma/theme.h>
#include <plasma/applet.h>

namespace Plasma
{

// Any attempt to generate a frame whose width or height is larger than this
// will be rejected
static const int MAX_FRAME_SIZE = 100000;

FrameSvg::FrameSvg(QObject *parent)
    : Svg(parent),
      d(new FrameSvgPrivate(this))
{
    connect(this, SIGNAL(repaintNeeded()), this, SLOT(updateNeeded()));
    d->frames.insert(QString(), new FrameData());
}

FrameSvg::~FrameSvg()
{
    delete d;
}

void FrameSvg::setImagePath(const QString &path)
{
    if (path == imagePath()) {
        return;
    }

    Svg::setImagePath(path);
    setContainsMultipleImages(true);

    clearCache();
    d->updateAndSignalSizes();
}

void FrameSvg::setEnabledBorders(const EnabledBorders borders)
{
    if (borders == d->frames[d->prefix]->enabledBorders) {
        return;
    }

    d->frames[d->prefix]->enabledBorders = borders;
    d->updateAndSignalSizes();
}

FrameSvg::EnabledBorders FrameSvg::enabledBorders() const
{
    if (d->frames.isEmpty()) {
        return NoBorder;
    }

    QHash<QString, FrameData*>::const_iterator it = d->frames.constFind(d->prefix);

    if (it != d->frames.constEnd()) {
        return it.value()->enabledBorders;
    } else {
        return NoBorder;
    }
}

void FrameSvg::setElementPrefix(Plasma::Location location)
{
    switch (location) {
        case TopEdge:
            setElementPrefix("north");
            break;
        case BottomEdge:
            setElementPrefix("south");
            break;
        case LeftEdge:
            setElementPrefix("west");
            break;
        case RightEdge:
            setElementPrefix("east");
            break;
        default:
            setElementPrefix(QString());
            break;
    }

    d->location = location;
}

void FrameSvg::setElementPrefix(const QString &prefix)
{
    const QString oldPrefix(d->prefix);

    if (!hasElement(prefix + "-center")) {
        d->prefix.clear();
    } else {
        d->prefix = prefix;
        if (!d->prefix.isEmpty()) {
            d->prefix += '-';
        }
    }

    FrameData *oldFrameData = d->frames.value(oldPrefix);
    if (oldPrefix == d->prefix && oldFrameData) {
        return;
    }

    if (!d->frames.contains(d->prefix)) {
        if (oldFrameData) {
            d->frames.insert(d->prefix, new FrameData(*oldFrameData));
        } else {
            d->frames.insert(d->prefix, new FrameData());
        }

        d->updateSizes();
    }

    if (!d->cacheAll) {
        delete d->frames[oldPrefix];
        d->frames.remove(oldPrefix);
    }

    d->location = Floating;
}

bool FrameSvg::hasElementPrefix(const QString & prefix) const
{
    //for now it simply checks if a center element exists,
    //because it could make sense for certain themes to not have all the elements
    if (prefix.isEmpty()) {
        return hasElement("center");
    } else {
        return hasElement(prefix + "-center");
    }
}

bool FrameSvg::hasElementPrefix(Plasma::Location location) const
{
    switch (location) {
        case TopEdge:
            return hasElementPrefix("north");
            break;
        case BottomEdge:
            return hasElementPrefix("south");
            break;
        case LeftEdge:
            return hasElementPrefix("west");
            break;
        case RightEdge:
            return hasElementPrefix("east");
            break;
        default:
            return hasElementPrefix(QString());
            break;
    }
}

QString FrameSvg::prefix()
{
    if (d->prefix.isEmpty()) {
        return d->prefix;
    }

    return d->prefix.left(d->prefix.size() - 1);
}

void FrameSvg::resizeFrame(const QSizeF &size)
{
    if (size.isEmpty()) {
        kWarning() << "Invalid size" << size;
        return;
    }

    if (size == d->frames[d->prefix]->frameSize) {
        return;
    }

    d->updateSizes();
    d->frames[d->prefix]->frameSize = size.toSize();
}

QSizeF FrameSvg::frameSize() const
{
    QHash<QString, FrameData*>::const_iterator it = d->frames.constFind(d->prefix);

    if (it == d->frames.constEnd()) {
        return QSize(-1, -1);
    } else {
        return d->frameSize(it.value());
    }
}

qreal FrameSvg::marginSize(const Plasma::MarginEdge edge) const
{
    if (d->frames[d->prefix]->noBorderPadding) {
        return .0;
    }

    switch (edge) {
    case Plasma::TopMargin:
        return d->frames[d->prefix]->topMargin;
    break;

    case Plasma::LeftMargin:
        return d->frames[d->prefix]->leftMargin;
    break;

    case Plasma::RightMargin:
        return d->frames[d->prefix]->rightMargin;
    break;

    //Plasma::BottomMargin
    default:
        return d->frames[d->prefix]->bottomMargin;
    break;
    }
}

void FrameSvg::getMargins(qreal &left, qreal &top, qreal &right, qreal &bottom) const
{
    FrameData *frame = d->frames[d->prefix];

    if (frame->noBorderPadding) {
        left = top = right = bottom = 0;
        return;
    }

    top = frame->topMargin;
    left = frame->leftMargin;
    right = frame->rightMargin;
    bottom = frame->bottomMargin;
}

QRectF FrameSvg::contentsRect() const
{
    QSizeF size(frameSize());

    if (size.isValid()) {
        QRectF rect(QPointF(0, 0), size);
        FrameData *frame = d->frames[d->prefix];

        return rect.adjusted(frame->leftMargin, frame->topMargin,
                             -frame->rightMargin, -frame->bottomMargin);
    } else {
        return QRectF();
    }
}

QPixmap FrameSvg::alphaMask() const
{
    //FIXME: the distinction between overlay and 
    return d->alphaMask();
}

QRegion FrameSvg::mask() const
{
    FrameData *frame = d->frames[d->prefix];
    frame->cachedMask = QRegion(QBitmap(d->alphaMask().alphaChannel().createMaskFromColor(Qt::black)));
    return frame->cachedMask;
}

void FrameSvg::setCacheAllRenderedFrames(bool cache)
{
    if (d->cacheAll && !cache) {
        clearCache();
    }

    d->cacheAll = cache;
}

bool FrameSvg::cacheAllRenderedFrames() const
{
    return d->cacheAll;
}

void FrameSvg::clearCache()
{
    FrameData *frame = d->frames[d->prefix];

    // delete all the frames that aren't this one
    QMutableHashIterator<QString, FrameData*> it(d->frames);
    while (it.hasNext()) {
        FrameData *p = it.next().value();

        if (frame != p) {
            //TODO: should we clear from the pixmap cache as well?
            delete p;
            it.remove();
        }
    }
}

QPixmap FrameSvg::framePixmap()
{
    FrameData *frame = d->frames[d->prefix];
    if (frame->cachedBackground.isNull()) {
        d->generateBackground(frame);
        if (frame->cachedBackground.isNull()) {
            return QPixmap();
        }
    }


    return frame->cachedBackground;
}

void FrameSvg::paintFrame(QPainter *painter, const QRectF &target, const QRectF &source)
{
    FrameData *frame = d->frames[d->prefix];
    if (frame->cachedBackground.isNull()) {
        d->generateBackground(frame);
        if (frame->cachedBackground.isNull()) {
            return;
        }
    }

    painter->drawPixmap(target, frame->cachedBackground, source.isValid() ? source : target);
}

void FrameSvg::paintFrame(QPainter *painter, const QPointF &pos)
{
    FrameData *frame = d->frames[d->prefix];
    if (frame->cachedBackground.isNull()) {
        d->generateBackground(frame);
        if (frame->cachedBackground.isNull()) {
            return;
        }
    }

    painter->drawPixmap(pos, frame->cachedBackground);
}

QPixmap FrameSvgPrivate::alphaMask()
{
    FrameData *frame = frames[prefix];
    QString maskPrefix;

    if (q->hasElement("mask-" + prefix + "center")) {
        maskPrefix = "mask-";
    }

    if (!maskPrefix.isNull()) {
        QString oldPrefix = prefix;

        // We are setting the prefix only temporary to generate
        // the needed mask image
        prefix = maskPrefix + oldPrefix;

        if (!frames.contains(prefix)) {
            frames.insert(prefix, new FrameData(*(frames[oldPrefix])));
            updateSizes();
        }

        FrameData *maskFrame = frames[prefix];
        if (maskFrame->cachedBackground.isNull() || maskFrame->frameSize != frameSize(frame)) {
            maskFrame->frameSize = frameSize(frame).toSize();
            maskFrame->cachedBackground = QPixmap();

            generateBackground(maskFrame);
            if (maskFrame->cachedBackground.isNull()) {
                return QPixmap();
            }
        }

        prefix = oldPrefix;
        return maskFrame->cachedBackground;
    } else {
        if (frame->cachedBackground.isNull()) {
            generateBackground(frame);
            if (frame->cachedBackground.isNull()) {
                return QPixmap();
            }
        }
        return frame->cachedBackground;
    }
}

void FrameSvgPrivate::generateBackground(FrameData *frame)
{
    if (!frame->cachedBackground.isNull()) {
        return;
    }

    const QString id = cacheId(frame, prefix);

    Theme *theme = Theme::defaultTheme();
    bool frameCached = !frame->cachedBackground.isNull();
    bool overlayCached = false;
    const bool overlayAvailable = !prefix.startsWith("mask-") && q->hasElement(prefix + "overlay");
    QPixmap overlay;
    if (q->isUsingRenderingCache()) {
        frameCached = theme->findInCache(id, frame->cachedBackground) && !frame->cachedBackground.isNull();

        if (overlayAvailable) {
            overlayCached = theme->findInCache("overlay_" + id, overlay) && !overlay.isNull();
        }
    }

    if (!frameCached) {
        generateFrameBackground(frame);
    }

    //Overlays
    QSize overlaySize;
    QPoint actualOverlayPos = QPoint(0, 0);
    if (overlayAvailable && !overlayCached) {
        QPoint pos = QPoint(0, 0);
        overlaySize = q->elementSize(prefix+"overlay");

        //Random pos, stretched and tiled are mutually exclusive
        if (q->hasElement(prefix + "hint-overlay-random-pos")) {
            actualOverlayPos = overlayPos;
        } else if (q->hasElement(prefix + "hint-overlay-pos-right")) {
            actualOverlayPos.setX(frame->frameSize.width() - overlaySize.width());
        } else if (q->hasElement(prefix + "hint-overlay-pos-bottom")) {
            actualOverlayPos.setY(frame->frameSize.height() - overlaySize.height());
        //Stretched or Tiled?
        } else if (q->hasElement(prefix + "hint-overlay-stretch")) {
            overlaySize = frameSize(frame).toSize();
        } else {
            if (q->hasElement(prefix + "hint-overlay-tile-horizontal")) {
                overlaySize.setWidth(frameSize(frame).width());
            }
            if (q->hasElement(prefix + "hint-overlay-tile-vertical")) {
                overlaySize.setHeight(frameSize(frame).height());
            }
        }

        overlay = alphaMask();
        QPainter overlayPainter(&overlay);
        overlayPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        //Tiling?
        if (q->hasElement(prefix+"hint-overlay-tile-horizontal") ||
            q->hasElement(prefix+"hint-overlay-tile-vertical")) {

            QSize s = q->size();
            q->resize(q->elementSize(prefix+"overlay"));

            overlayPainter.drawTiledPixmap(QRect(QPoint(0,0), overlaySize), q->pixmap(prefix+"overlay"));
            q->resize(s);
        } else {
            q->paint(&overlayPainter, QRect(actualOverlayPos, overlaySize), prefix+"overlay");
        }

        overlayPainter.end();
    }

    if (!frameCached) {
        cacheFrame(prefix, frame->cachedBackground, overlayCached ? overlay : QPixmap());
    }

    if (!overlay.isNull()) {
        QPainter p(&frame->cachedBackground);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.drawPixmap(actualOverlayPos, overlay, QRect(actualOverlayPos, overlaySize));
    }
}

void FrameSvgPrivate::generateFrameBackground(FrameData *frame)
{
    //kDebug() << "generating background";
    const QSizeF size = frameSize(frame);
    const int topWidth = q->elementSize(prefix + "top").width();
    const int leftHeight = q->elementSize(prefix + "left").height();
    const int topOffset = 0;
    const int leftOffset = 0;


    if (!size.isValid()) {
        kWarning() << "Invalid frame size" << size;
        return;
    }
    if (size.width() >= MAX_FRAME_SIZE || size.height() >= MAX_FRAME_SIZE) {
        kWarning() << "Not generating frame background for a size whose width or height is more than" << MAX_FRAME_SIZE << size;
        return;
    }

    const int contentWidth = size.width() - frame->leftWidth  - frame->rightWidth;
    const int contentHeight = size.height() - frame->topHeight  - frame->bottomHeight;
    int contentTop = 0;
    int contentLeft = 0;
    int rightOffset = contentWidth;
    int bottomOffset = contentHeight;

    frame->cachedBackground = QPixmap(frame->leftWidth + contentWidth + frame->rightWidth,
                                      frame->topHeight + contentHeight + frame->bottomHeight);
    frame->cachedBackground.fill(Qt::transparent);
    QPainter p(&frame->cachedBackground);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    //CENTER
    if (frame->tileCenter) {
        if (contentHeight > 0 && contentWidth > 0) {
            const int centerTileHeight = q->elementSize(prefix + "center").height();
            const int centerTileWidth = q->elementSize(prefix + "center").width();
            QPixmap center(centerTileWidth, centerTileHeight);
            center.fill(Qt::transparent);

            {
                QPainter centerPainter(&center);
                centerPainter.setCompositionMode(QPainter::CompositionMode_Source);
                q->paint(&centerPainter, QRect(QPoint(0, 0), q->elementSize(prefix + "center")), prefix + "center");
            }

            if (frame->composeOverBorder) {
                p.drawTiledPixmap(QRect(QPoint(0, 0), size.toSize()), center);
            } else {
                p.drawTiledPixmap(QRect(frame->leftWidth, frame->topHeight,
                                        contentWidth, contentHeight), center);
            }
        }
    } else {
        if (contentHeight > 0 && contentWidth > 0) {
            if (frame->composeOverBorder) {
                q->paint(&p, QRect(QPoint(0, 0), size.toSize()),
                               prefix + "center");
            } else {
                q->paint(&p, QRect(frame->leftWidth, frame->topHeight,
                                contentWidth, contentHeight),
                                prefix + "center");
            }
        }
    }

    if (frame->composeOverBorder) {
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.drawPixmap(QRect(QPoint(0, 0), size.toSize()), alphaMask());
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    }

    if (frame->enabledBorders & FrameSvg::LeftBorder && q->hasElement(prefix + "left")) {
        rightOffset += frame->leftWidth;
    }

    // Corners
    if (frame->enabledBorders & FrameSvg::TopBorder && q->hasElement(prefix + "top")) {
        contentTop = frame->topHeight;
        bottomOffset += frame->topHeight;

        if (q->hasElement(prefix + "topleft") && frame->enabledBorders & FrameSvg::LeftBorder) {
            q->paint(&p, QRect(leftOffset, topOffset, frame->leftWidth, frame->topHeight), prefix + "topleft");

            contentLeft = frame->leftWidth;
        }

        if (q->hasElement(prefix + "topright") && frame->enabledBorders & FrameSvg::RightBorder) {
            q->paint(&p, QRect(rightOffset, topOffset, frame->rightWidth, frame->topHeight), prefix + "topright");
        }
    }

    if (frame->enabledBorders & FrameSvg::BottomBorder && q->hasElement(prefix + "bottom")) {
        if (q->hasElement(prefix + "bottomleft") && frame->enabledBorders & FrameSvg::LeftBorder) {
            q->paint(&p, QRect(leftOffset, bottomOffset, frame->leftWidth, frame->bottomHeight), prefix + "bottomleft");

            contentLeft = frame->leftWidth;
        }

        if (frame->enabledBorders & FrameSvg::RightBorder && q->hasElement(prefix + "bottomright")) {
            q->paint(&p, QRect(rightOffset, bottomOffset, frame->rightWidth, frame->bottomHeight), prefix + "bottomright");
        }
    }

    // Sides
    if (frame->stretchBorders) {
        if (frame->enabledBorders & FrameSvg::LeftBorder || frame->enabledBorders & FrameSvg::RightBorder) {
            if (q->hasElement(prefix + "left") &&
                frame->enabledBorders & FrameSvg::LeftBorder) {
                q->paint(&p, QRect(leftOffset, contentTop, frame->leftWidth, contentHeight), prefix + "left");
            }

            if (q->hasElement(prefix + "right") &&
                frame->enabledBorders & FrameSvg::RightBorder) {
                q->paint(&p, QRect(rightOffset, contentTop, frame->rightWidth, contentHeight), prefix + "right");
            }
        }

        if (frame->enabledBorders & FrameSvg::TopBorder || frame->enabledBorders & FrameSvg::BottomBorder) {
            if (frame->enabledBorders & FrameSvg::TopBorder && q->hasElement(prefix + "top")) {
                q->paint(&p, QRect(contentLeft, topOffset, contentWidth, frame->topHeight), prefix + "top");
            }

            if (frame->enabledBorders & FrameSvg::BottomBorder && q->hasElement(prefix + "bottom")) {
                q->paint(&p, QRect(contentLeft, bottomOffset, contentWidth, frame->bottomHeight), prefix + "bottom");
            }
        }
    } else {
        if (frame->enabledBorders & FrameSvg::LeftBorder && q->hasElement(prefix + "left")
                && leftHeight > 0 && frame->leftWidth > 0) {
            QPixmap left(frame->leftWidth, leftHeight);
            left.fill(Qt::transparent);

            QPainter sidePainter(&left);
            sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
            q->paint(&sidePainter, QRect(QPoint(0, 0), left.size()), prefix + "left");

            p.drawTiledPixmap(QRect(leftOffset, contentTop, frame->leftWidth, contentHeight), left);
        }

        if (frame->enabledBorders & FrameSvg::RightBorder && q->hasElement(prefix + "right") &&
                leftHeight > 0 && frame->rightWidth > 0) {
            QPixmap right(frame->rightWidth, leftHeight);
            right.fill(Qt::transparent);

            QPainter sidePainter(&right);
            sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
            q->paint(&sidePainter, QRect(QPoint(0, 0), right.size()), prefix + "right");

            p.drawTiledPixmap(QRect(rightOffset, contentTop, frame->rightWidth, contentHeight), right);
        }

        if (frame->enabledBorders & FrameSvg::TopBorder && q->hasElement(prefix + "top")
                && topWidth > 0 && frame->topHeight > 0) {
            QPixmap top(topWidth, frame->topHeight);
            top.fill(Qt::transparent);

            QPainter sidePainter(&top);
            sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
            q->paint(&sidePainter, QRect(QPoint(0, 0), top.size()), prefix + "top");

            p.drawTiledPixmap(QRect(contentLeft, topOffset, contentWidth, frame->topHeight), top);
        }

        if (frame->enabledBorders & FrameSvg::BottomBorder && q->hasElement(prefix + "bottom")
                && topWidth > 0 && frame->bottomHeight > 0) {
            QPixmap bottom(topWidth, frame->bottomHeight);
            bottom.fill(Qt::transparent);

            QPainter sidePainter(&bottom);
            sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
            q->paint(&sidePainter, QRect(QPoint(0, 0), bottom.size()), prefix + "bottom");

            p.drawTiledPixmap(QRect(contentLeft, bottomOffset, contentWidth, frame->bottomHeight), bottom);
        }
    }

}

QString FrameSvgPrivate::cacheId(FrameData *frame, const QString &prefixToSave) const
{
    const QSizeF size = frameSize(frame);
    return QString::fromLatin1("%5_%4_%3_%2_%1_").arg(frame->enabledBorders)
                                                 .arg(size.width())
                                                 .arg(size.height())
                                                 .arg(prefixToSave)
                                                 .arg(q->imagePath());
}

void FrameSvgPrivate::cacheFrame(const QString &prefixToSave, const QPixmap &background, const QPixmap &overlay)
{
    if (!q->isUsingRenderingCache()) {
        return;
    }

    //insert background
    FrameData *frame = frames.value(prefixToSave);

    if (!frame) {
        return;
    }

    const QString id = cacheId(frame, prefixToSave);

    //kDebug()<<"Saving to cache frame"<<id;

    Theme::defaultTheme()->insertIntoCache(id, background, QString::number((qint64)q, 16)+prefixToSave);

    if (!overlay.isNull()) {
        //insert overlay
        Theme::defaultTheme()->insertIntoCache("overlay_" + id, overlay, QString::number((qint64)q, 16) + prefixToSave + "overlay");
    }
}

void FrameSvgPrivate::updateSizes() const
{
    //kDebug() << "!!!!!!!!!!!!!!!!!!!!!! updating sizes" << prefix;
    FrameData *frame = frames[prefix];
    Q_ASSERT(frame);

    QSize s = q->size();
    q->resize();
    frame->cachedBackground = QPixmap();
    frame->cachedMask = QRegion();

    if (frame->enabledBorders & FrameSvg::TopBorder) {
        frame->topHeight = q->elementSize(prefix + "top").height();

        if (q->hasElement(prefix + "hint-top-margin")) {
            frame->topMargin = q->elementSize(prefix + "hint-top-margin").height();
        } else {
            frame->topMargin = frame->topHeight;
        }
    } else {
        frame->topMargin = frame->topHeight = 0;
    }

    if (frame->enabledBorders & FrameSvg::LeftBorder) {
        frame->leftWidth = q->elementSize(prefix + "left").width();

        if (q->hasElement(prefix + "hint-left-margin")) {
            frame->leftMargin = q->elementSize(prefix + "hint-left-margin").width();
        } else {
            frame->leftMargin = frame->leftWidth;
        }
    } else {
        frame->leftMargin = frame->leftWidth = 0;
    }

    if (frame->enabledBorders & FrameSvg::RightBorder) {
        frame->rightWidth = q->elementSize(prefix + "right").width();

        if (q->hasElement(prefix + "hint-right-margin")) {
            frame->rightMargin = q->elementSize(prefix + "hint-right-margin").width();
        } else {
            frame->rightMargin = frame->rightWidth;
        }
    } else {
        frame->rightMargin = frame->rightWidth = 0;
    }

    if (frame->enabledBorders & FrameSvg::BottomBorder) {
        frame->bottomHeight = q->elementSize(prefix + "bottom").height();

        if (q->hasElement(prefix + "hint-bottom-margin")) {
            frame->bottomMargin = q->elementSize(prefix + "hint-bottom-margin").height();
        } else {
            frame->bottomMargin = frame->bottomHeight;
        }
    } else {
        frame->bottomMargin = frame->bottomHeight = 0;
    }

    frame->composeOverBorder = (q->hasElement(prefix + "hint-compose-over-border") &&
                                q->hasElement(prefix + "mask-center"));

    //since it's rectangular, topWidth and bottomWidth must be the same
    //the ones that don't have a prefix is for retrocompatibility
    frame->tileCenter = q->hasElement("hint-tile-center");
    frame->noBorderPadding = q->hasElement("hint-no-border-padding");
    frame->stretchBorders = q->hasElement("hint-stretch-borders");
    q->resize(s);
}

void FrameSvgPrivate::updateNeeded()
{
    q->clearCache();
    updateSizes();
}

void FrameSvgPrivate::updateAndSignalSizes()
{
    updateSizes();
    emit q->repaintNeeded();
}

QSizeF FrameSvgPrivate::frameSize(FrameData *frame) const
{
    if (!frame->frameSize.isValid()) {
        updateSizes();
        frame->frameSize = q->size();
    }

    return frame->frameSize;
}


} // Plasma namespace

#include "framesvg.moc"
