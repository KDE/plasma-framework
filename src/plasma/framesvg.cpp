/*
 *   Copyright 2008-2010 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008-2010 Marco Martin <notmart@gmail.com>
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

#include <QAtomicInt>
#include <QBitmap>
#include <QCryptographicHash>
#include <QPainter>
#include <QRegion>
#include <QSize>
#include <QStringBuilder>
#include <QTimer>

#include <QDebug>

#include "theme.h"
#include "private/svg_p.h"
#include "private/framesvg_helpers.h"
#include "debug_p.h"

namespace Plasma
{

QHash<ThemePrivate *, QHash<QString, QWeakPointer<FrameData>> > FrameSvgPrivate::s_sharedFrames;

// Any attempt to generate a frame whose width or height is larger than this
// will be rejected
static const int MAX_FRAME_SIZE = 100000;

FrameData::~FrameData()
{
    FrameSvgPrivate::s_sharedFrames[theme].remove(cacheId);
}

FrameSvg::FrameSvg(QObject *parent)
    : Svg(parent),
      d(new FrameSvgPrivate(this))
{
    connect(this, &FrameSvg::repaintNeeded, this, std::bind(&FrameSvgPrivate::updateNeeded, d));
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

    clearCache();

    setContainsMultipleImages(true);
    Svg::d->setImagePath(path);
    if (!d->repaintBlocked) {
        d->updateFrameData();
    }
}

void FrameSvg::setEnabledBorders(const EnabledBorders borders)
{
    if (borders == d->enabledBorders) {
        return;
    }

    d->enabledBorders = borders;

    if (!d->repaintBlocked) {
        d->updateFrameData();
    }
}

FrameSvg::EnabledBorders FrameSvg::enabledBorders() const
{
    return d->enabledBorders;
}

void FrameSvg::setElementPrefix(Plasma::Types::Location location)
{
    switch (location) {
    case Types::TopEdge:
        setElementPrefix(QStringLiteral("north"));
        break;
    case Types::BottomEdge:
        setElementPrefix(QStringLiteral("south"));
        break;
    case Types::LeftEdge:
        setElementPrefix(QStringLiteral("west"));
        break;
    case Types::RightEdge:
        setElementPrefix(QStringLiteral("east"));
        break;
    default:
        setElementPrefix(QString());
        break;
    }

    d->location = location;
}

void FrameSvg::setElementPrefix(const QString &prefix)
{
    if (!hasElement(prefix % QLatin1String("-center"))) {
        d->prefix.clear();
    } else {
        d->prefix = prefix;
        if (!d->prefix.isEmpty()) {
            d->prefix += QLatin1Char('-');
        }
    }
    d->requestedPrefix = prefix;

    d->location = Types::Floating;

    if (!d->repaintBlocked) {
        d->updateFrameData();
    }
}

bool FrameSvg::hasElementPrefix(const QString &prefix) const
{
    //for now it simply checks if a center element exists,
    //because it could make sense for certain themes to not have all the elements
    if (prefix.isEmpty()) {
        return hasElement(QStringLiteral("center"));
    }
    if (prefix.endsWith(QLatin1Char('-'))) {
        return hasElement(prefix % QLatin1String("center"));
    }

    return hasElement(prefix % QLatin1String("-center"));
}

bool FrameSvg::hasElementPrefix(Plasma::Types::Location location) const
{
    switch (location) {
    case Types::TopEdge:
        return hasElementPrefix(QStringLiteral("north"));
    case Types::BottomEdge:
        return hasElementPrefix(QStringLiteral("south"));
    case Types::LeftEdge:
        return hasElementPrefix(QStringLiteral("west"));
    case Types::RightEdge:
        return hasElementPrefix(QStringLiteral("east"));
    default:
        return hasElementPrefix(QString());
    }
}

QString FrameSvg::prefix()
{
    return d->requestedPrefix;
}

void FrameSvg::resizeFrame(const QSizeF &size)
{
    if (imagePath().isEmpty()) {
        return;
    }

    if (size.isEmpty()) {
#ifndef NDEBUG
        // qCDebug(LOG_PLASMA) << "Invalid size" << size;
#endif
        return;
    }

    if (d->frame && size.toSize() == d->frame->frameSize) {
        return;
    }
    d->pendingFrameSize = size.toSize();

    if (!d->repaintBlocked) {
        d->updateFrameData(FrameSvgPrivate::UpdateFrame);
    }
}

QSizeF FrameSvg::frameSize() const
{
    if (!d->frame) {
        return QSize(-1, -1);
    } else {
        return d->frameSize(d->frame.data());
    }
}

qreal FrameSvg::marginSize(const Plasma::Types::MarginEdge edge) const
{
    if (!d->frame) {
        return .0;
    }

    if (d->frame->noBorderPadding) {
        return .0;
    }

    switch (edge) {
    case Plasma::Types::TopMargin:
        return d->frame->topMargin;

    case Plasma::Types::LeftMargin:
        return d->frame->leftMargin;

    case Plasma::Types::RightMargin:
        return d->frame->rightMargin;

    //Plasma::BottomMargin
    default:
        return d->frame->bottomMargin;
    }
}

qreal FrameSvg::fixedMarginSize(const Plasma::Types::MarginEdge edge) const
{
    if (!d->frame) {
        return .0;
    }

    if (d->frame->noBorderPadding) {
        return .0;
    }

    switch (edge) {
    case Plasma::Types::TopMargin:
        return d->frame->fixedTopMargin;

    case Plasma::Types::LeftMargin:
        return d->frame->fixedLeftMargin;

    case Plasma::Types::RightMargin:
        return d->frame->fixedRightMargin;

    //Plasma::BottomMargin
    default:
        return d->frame->fixedBottomMargin;
    }
}

void FrameSvg::getMargins(qreal &left, qreal &top, qreal &right, qreal &bottom) const
{
    if (!d->frame || d->frame->noBorderPadding) {
        left = top = right = bottom = 0;
        return;
    }

    top = d->frame->topMargin;
    left = d->frame->leftMargin;
    right = d->frame->rightMargin;
    bottom = d->frame->bottomMargin;
}

void FrameSvg::getFixedMargins(qreal &left, qreal &top, qreal &right, qreal &bottom) const
{
    if (!d->frame || d->frame->noBorderPadding) {
        left = top = right = bottom = 0;
        return;
    }

    top = d->frame->fixedTopMargin;
    left = d->frame->fixedLeftMargin;
    right = d->frame->fixedRightMargin;
    bottom = d->frame->fixedBottomMargin;
}

QRectF FrameSvg::contentsRect() const
{
    if (d->frame) {
        QRectF rect(QPoint(0,0), d->frame->frameSize);
        return rect.adjusted(d->frame->leftMargin, d->frame->topMargin, -d->frame->rightMargin, -d->frame->bottomMargin);
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
    QRegion result;
    if (!d->frame) {
        return result;
    }

    QString id = d->cacheId(d->frame.data(), QString());

    QRegion* obj = d->frame->cachedMasks.object(id);

    if (!obj) {
        obj = new QRegion(QBitmap(d->alphaMask().mask()));
        result = *obj;
        d->frame->cachedMasks.insert(id, obj);
    }
    else {
        result = *obj;
    }
    return result;
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
    if (d->frame) {
        d->frame->cachedBackground = QPixmap();
        d->frame->cachedMasks.clear();
    }
    if (d->maskFrame) {
        d->maskFrame->cachedBackground = QPixmap();
        d->maskFrame->cachedMasks.clear();
    }
}

QPixmap FrameSvg::framePixmap()
{
    if (d->frame->cachedBackground.isNull()) {
        d->generateBackground(d->frame);
    }

    return d->frame->cachedBackground;
}

void FrameSvg::paintFrame(QPainter *painter, const QRectF &target, const QRectF &source)
{
    if (d->frame->cachedBackground.isNull()) {
        d->generateBackground(d->frame);
        if (d->frame->cachedBackground.isNull()) {
            return;
        }
    }

    painter->drawPixmap(target, d->frame->cachedBackground, source.isValid() ? source : target);
}

void FrameSvg::paintFrame(QPainter *painter, const QPointF &pos)
{
    if (d->frame->cachedBackground.isNull()) {
        d->generateBackground(d->frame);
        if (d->frame->cachedBackground.isNull()) {
            return;
        }
    }

    painter->drawPixmap(pos, d->frame->cachedBackground);
}

//#define DEBUG_FRAMESVG_CACHE
FrameSvgPrivate::~FrameSvgPrivate() = default;

QPixmap FrameSvgPrivate::alphaMask()
{
    QString maskPrefix;

    if (q->hasElement(QLatin1String("mask-") % prefix % QLatin1String("center"))) {
        maskPrefix = QStringLiteral("mask-");
    }

    if (maskPrefix.isNull()) {
        if (frame->cachedBackground.isNull()) {
            generateBackground(frame);
        }
        return frame->cachedBackground;
    }

    // We are setting the prefix only temporary to generate
    // the needed mask image
    const QString maskRequestedPrefix = requestedPrefix.isEmpty() ? QStringLiteral("mask") : maskPrefix % requestedPrefix;
    maskPrefix = maskPrefix % prefix;

    if (!maskFrame) {
        maskFrame = lookupOrCreateMaskFrame(frame, maskPrefix, maskRequestedPrefix);
        if (!maskFrame->cachedBackground.isNull()) {
            return maskFrame->cachedBackground;
        }
        updateSizes(maskFrame);
        generateBackground(maskFrame);
        return maskFrame->cachedBackground;
    }

    const bool shouldUpdate = maskFrame->enabledBorders != frame->enabledBorders
        || maskFrame->frameSize != frameSize(frame.data())
        || maskFrame->imagePath != frame->imagePath;
    if (shouldUpdate) {
        maskFrame = lookupOrCreateMaskFrame(frame, maskPrefix, maskRequestedPrefix);
        if (!maskFrame->cachedBackground.isNull()) {
            return maskFrame->cachedBackground;
        }
        updateSizes(maskFrame);
    }

    if (maskFrame->cachedBackground.isNull()) {
        generateBackground(maskFrame);
    }

    return maskFrame->cachedBackground;
}

QSharedPointer<FrameData> FrameSvgPrivate::lookupOrCreateMaskFrame(const QSharedPointer<FrameData> &frame, const QString &maskPrefix, const QString &maskRequestedPrefix)
{
    const QString key = cacheId(frame.data(), maskPrefix);
    QSharedPointer<FrameData> mask = s_sharedFrames[q->theme()->d].value(key);

    // See if we can find a suitable candidate in the shared frames.
    // If there is one, use it.
    if (mask) {
        return mask;
    }

    mask.reset(new FrameData(*frame.data(), q));
    mask->prefix = maskPrefix;
    mask->requestedPrefix = maskRequestedPrefix;
    mask->theme = q->theme()->d;
    mask->imagePath = frame->imagePath;
    mask->enabledBorders = frame->enabledBorders;
    mask->frameSize = frameSize(frame).toSize();
    mask->cacheId = key;
    s_sharedFrames[q->theme()->d].insert(key, mask);

    return mask;
}

void FrameSvgPrivate::generateBackground(const QSharedPointer<FrameData> &frame)
{
    if (!frame->cachedBackground.isNull() || !q->hasElementPrefix(frame->prefix)) {
        return;
    }

    const QString id = cacheId(frame.data(), frame->prefix);

    bool frameCached = !frame->cachedBackground.isNull();
    bool overlayCached = false;
    const bool overlayAvailable = !frame->prefix.startsWith(QLatin1String("mask-")) && q->hasElement(frame->prefix % QLatin1String("overlay"));
    QPixmap overlay;
    if (q->isUsingRenderingCache()) {
        frameCached = q->theme()->findInCache(id, frame->cachedBackground) && !frame->cachedBackground.isNull();

        if (overlayAvailable) {
            overlayCached = q->theme()->findInCache(QLatin1String("overlay_") % id, overlay) && !overlay.isNull();
        }
    }

    if (!frameCached) {
        generateFrameBackground(frame);
    }

    //Overlays
    QSize overlaySize;
    QPoint actualOverlayPos = QPoint(0, 0);
    if (overlayAvailable && !overlayCached) {
        overlaySize = q->elementSize(frame->prefix % QLatin1String("overlay"));

        if (q->hasElement(frame->prefix % QLatin1String("hint-overlay-pos-right"))) {
            actualOverlayPos.setX(frame->frameSize.width() - overlaySize.width());
        } else if (q->hasElement(frame->prefix % QLatin1String("hint-overlay-pos-bottom"))) {
            actualOverlayPos.setY(frame->frameSize.height() - overlaySize.height());
            //Stretched or Tiled?
        } else if (q->hasElement(frame->prefix % QLatin1String("hint-overlay-stretch"))) {
            overlaySize = frameSize(frame).toSize();
        } else {
            if (q->hasElement(frame->prefix % QLatin1String("hint-overlay-tile-horizontal"))) {
                overlaySize.setWidth(frameSize(frame).width());
            }
            if (q->hasElement(frame->prefix % QLatin1String("hint-overlay-tile-vertical"))) {
                overlaySize.setHeight(frameSize(frame).height());
            }
        }

        overlay = alphaMask();
        QPainter overlayPainter(&overlay);
        overlayPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        //Tiling?
        if (q->hasElement(frame->prefix % QLatin1String("hint-overlay-tile-horizontal")) ||
                q->hasElement(frame->prefix % QLatin1String("hint-overlay-tile-vertical"))) {

            QSize s = q->size();
            q->resize(q->elementSize(frame->prefix % QLatin1String("overlay")));

            overlayPainter.drawTiledPixmap(QRect(QPoint(0, 0), overlaySize), q->pixmap(frame->prefix % QLatin1String("overlay")));
            q->resize(s);
        } else {
            q->paint(&overlayPainter, QRect(actualOverlayPos, overlaySize), frame->prefix % QLatin1String("overlay"));
        }

        overlayPainter.end();
    }

    if (!frameCached) {
        cacheFrame(frame->prefix, frame->cachedBackground, overlayCached ? overlay : QPixmap());
    }

    if (!overlay.isNull()) {
        QPainter p(&frame->cachedBackground);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.drawPixmap(actualOverlayPos, overlay, QRect(actualOverlayPos, overlaySize));
    }
}

void FrameSvgPrivate::generateFrameBackground(const QSharedPointer<FrameData> &frame)
{
    //qCDebug(LOG_PLASMA) << "generating background";
    const QSize size = frameSize(frame).toSize() * q->devicePixelRatio();

    if (!size.isValid()) {
#ifndef NDEBUG
        // qCDebug(LOG_PLASMA) << "Invalid frame size" << size;
#endif
        return;
    }
    if (size.width() >= MAX_FRAME_SIZE || size.height() >= MAX_FRAME_SIZE) {
        qCWarning(LOG_PLASMA) << "Not generating frame background for a size whose width or height is more than" << MAX_FRAME_SIZE << size;
        return;
    }

    frame->cachedBackground = QPixmap(size);
    frame->cachedBackground.fill(Qt::transparent);
    QPainter p(&frame->cachedBackground);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    QRect contentRect = contentGeometry(frame, size);
    paintCenter(p, frame, contentRect, size);

    paintCorner(p, frame, FrameSvg::LeftBorder|FrameSvg::TopBorder, contentRect);
    paintCorner(p, frame, FrameSvg::RightBorder|FrameSvg::TopBorder, contentRect);
    paintCorner(p, frame, FrameSvg::LeftBorder|FrameSvg::BottomBorder, contentRect);
    paintCorner(p, frame, FrameSvg::RightBorder|FrameSvg::BottomBorder, contentRect);

    // Sides
    const int leftHeight = q->elementSize(frame->prefix % QLatin1String("left")).height();
    paintBorder(p, frame, FrameSvg::LeftBorder, QSize(frame->leftWidth, leftHeight) * q->devicePixelRatio(), contentRect);
    const int rightHeight = q->elementSize(frame->prefix % QLatin1String("right")).height();
    paintBorder(p, frame, FrameSvg::RightBorder, QSize(frame->rightWidth, rightHeight) * q->devicePixelRatio(), contentRect);

    const int topWidth = q->elementSize(frame->prefix % QLatin1String("top")).width();
    paintBorder(p, frame, FrameSvg::TopBorder, QSize(topWidth, frame->topHeight) * q->devicePixelRatio(), contentRect);
    const int bottomWidth = q->elementSize(frame->prefix % QLatin1String("bottom")).width();
    paintBorder(p, frame, FrameSvg::BottomBorder, QSize(bottomWidth, frame->bottomHeight) * q->devicePixelRatio(), contentRect);
    p.end();

    frame->cachedBackground.setDevicePixelRatio(q->devicePixelRatio());
}

QRect FrameSvgPrivate::contentGeometry(const QSharedPointer<FrameData> &frame, const QSize& size) const
{
    const QSize contentSize(size.width() - frame->leftWidth * q->devicePixelRatio() - frame->rightWidth * q->devicePixelRatio(),
                            size.height() - frame->topHeight * q->devicePixelRatio() - frame->bottomHeight * q->devicePixelRatio());
    QRect contentRect(QPoint(0,0), contentSize);
    if (frame->enabledBorders & FrameSvg::LeftBorder && q->hasElement(frame->prefix % QLatin1String("left"))) {
        contentRect.translate(frame->leftWidth * q->devicePixelRatio(), 0);
    }

    // Corners
    if (frame->enabledBorders & FrameSvg::TopBorder && q->hasElement(frame->prefix % QLatin1String("top"))) {
        contentRect.translate(0, frame->topHeight * q->devicePixelRatio());
    }
    return contentRect;
}

void FrameSvgPrivate::updateFrameData(UpdateType updateType)
{
    auto fd = frame;
    QString newKey;

    if (fd) {
        const QString oldKey = fd->cacheId;

        const QString oldPath = fd->imagePath;
        const FrameSvg::EnabledBorders oldBorders = fd->enabledBorders;
        const QSize currentSize = fd->frameSize;

        fd->enabledBorders = enabledBorders;
        fd->frameSize = pendingFrameSize;
        fd->imagePath = q->imagePath();

        newKey = cacheId(fd.data(), prefix);

        //reset frame to old values
        fd->enabledBorders = oldBorders;
        fd->frameSize = currentSize;
        fd->imagePath = oldPath;

        //FIXME: something more efficient than string comparison?
        if (oldKey == newKey) {
            return;
        }

        //qCDebug(LOG_PLASMA) << "looking for" << newKey;
        auto newFd = FrameSvgPrivate::s_sharedFrames[q->theme()->d].value(newKey);
        if (newFd) {
            //qCDebug(LOG_PLASMA) << "FOUND IT!" << newFd->refcount;
            // we've found a match, use that one
            Q_ASSERT(newKey == newFd.lock()->cacheId);
            frame = newFd;
            return;
        }

        fd.reset(new FrameData(*fd, q));
    } else {
        fd.reset(new FrameData(q, QString()));
    }

    frame = fd;
    fd->prefix = prefix;
    fd->requestedPrefix = requestedPrefix;
    //updateSizes();
    fd->enabledBorders = enabledBorders;
    fd->frameSize = pendingFrameSize;
    fd->imagePath = q->imagePath();
    //was fd just created empty now?
    if (newKey.isEmpty()) {
        newKey = cacheId(fd.data(), prefix);
    }

    // we know it isn't in s_sharedFrames due to the check above, so insert it now
    FrameSvgPrivate::s_sharedFrames[q->theme()->d].insert(newKey, fd);
    fd->cacheId = newKey;
    fd->theme = q->theme()->d;
    if (updateType == UpdateFrameAndMargins) {
        updateAndSignalSizes();
    } else {
        updateSizes(frame);
    }
}

void FrameSvgPrivate::paintCenter(QPainter& p, const QSharedPointer<FrameData> &frame, const QRect& contentRect, const QSize& fullSize)
{
    if (!contentRect.isEmpty()) {
        const QString centerElementId = frame->prefix % QLatin1String("center");
        if (frame->tileCenter) {
            QSize centerTileSize = q->elementSize(centerElementId);
            QPixmap center(centerTileSize);
            center.fill(Qt::transparent);

            QPainter centerPainter(&center);
            centerPainter.setCompositionMode(QPainter::CompositionMode_Source);
            q->paint(&centerPainter, QRect(QPoint(0, 0), centerTileSize),centerElementId);

            if (frame->composeOverBorder) {
                p.drawTiledPixmap(QRect(QPoint(0, 0), fullSize), center);
            } else {
                p.drawTiledPixmap(FrameSvgHelpers::sectionRect(FrameSvg::NoBorder, contentRect, fullSize * q->devicePixelRatio()), center);
            }
        } else {
            if (frame->composeOverBorder) {
                q->paint(&p, QRect(QPoint(0, 0), fullSize),
                         centerElementId);
            } else {
                q->paint(&p, FrameSvgHelpers::sectionRect(FrameSvg::NoBorder, contentRect, fullSize * q->devicePixelRatio()), centerElementId);
            }
        }
    }

    if (frame->composeOverBorder) {
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.drawPixmap(QRect(QPoint(0, 0), fullSize), alphaMask());
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    }
}

void FrameSvgPrivate::paintBorder(QPainter& p, const QSharedPointer<FrameData> &frame, const FrameSvg::EnabledBorders borders, const QSize& size, const QRect& contentRect) const
{
    QString side = frame->prefix % FrameSvgHelpers::borderToElementId(borders);
    if (frame->enabledBorders & borders && q->hasElement(side) && !size.isEmpty()) {
        if (frame->stretchBorders) {
            q->paint(&p, FrameSvgHelpers::sectionRect(borders, contentRect, frame->frameSize * q->devicePixelRatio()), side);
        } else {
            QPixmap px(size);
            px.fill(Qt::transparent);

            QPainter sidePainter(&px);
            sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
            q->paint(&sidePainter, QRect(QPoint(0, 0), size), side);

            p.drawTiledPixmap(FrameSvgHelpers::sectionRect(borders, contentRect, frame->frameSize * q->devicePixelRatio()), px);
        }
    }
}

void FrameSvgPrivate::paintCorner(QPainter& p, const QSharedPointer<FrameData> &frame, Plasma::FrameSvg::EnabledBorders border, const QRect& contentRect) const
{
    // Draw the corner only if both borders in both directions are enabled.
    if ((frame->enabledBorders & border) != border) {
        return;
    }
    const QString corner = frame->prefix % FrameSvgHelpers::borderToElementId(border);
    if (q->hasElement(corner)) {
        q->paint(&p, FrameSvgHelpers::sectionRect(border, contentRect, frame->frameSize * q->devicePixelRatio()), corner);
    }
}

QString FrameSvgPrivate::cacheId(FrameData *frame, const QString &prefixToSave) const
{
    const QSize size = frameSize(frame).toSize();
    const QLatin1Char s('_');
    return QString::number(frame->enabledBorders) % s % QString::number(size.width()) % s % QString::number(size.height()) % s % QString::number(q->scaleFactor()) % s % QString::number(q->devicePixelRatio()) % s % prefixToSave % s % frame->imagePath;
}

void FrameSvgPrivate::cacheFrame(const QString &prefixToSave, const QPixmap &background, const QPixmap &overlay)
{
    if (!q->isUsingRenderingCache()) {
        return;
    }

    //insert background
    if (!frame) {
        return;
    }

    const QString id = cacheId(frame.data(), prefixToSave);

    //qCDebug(LOG_PLASMA)<<"Saving to cache frame"<<id;

    q->theme()->insertIntoCache(id, background, QString::number((qint64)q, 16) % prefixToSave);

    if (!overlay.isNull()) {
        //insert overlay
        q->theme()->insertIntoCache(QLatin1String("overlay_") % id, overlay, QString::number((qint64)q, 16) % prefixToSave % QLatin1String("overlay"));
    }
}

void FrameSvgPrivate::updateSizes(FrameData *frame) const
{
    //qCDebug(LOG_PLASMA) << "!!!!!!!!!!!!!!!!!!!!!! updating sizes" << prefix;
    Q_ASSERT(frame);

    QSize s = q->size();
    q->resize();
    if (!frame->cachedBackground.isNull()) {
        frame->cachedBackground = QPixmap();
    }

    //This has the same size regardless the border is enabled or not
    frame->fixedTopHeight = q->elementSize(frame->prefix % QLatin1String("top")).height();

    if (q->hasElement(frame->prefix % QLatin1String("hint-top-margin"))) {
        frame->fixedTopMargin = q->elementSize(frame->prefix % QLatin1String("hint-top-margin")).height();
    } else {
        frame->fixedTopMargin = frame->fixedTopHeight;
    }

    //The same, but its size depends from the margin being enabled
    if (frame->enabledBorders & FrameSvg::TopBorder) {
        frame->topMargin = frame->fixedTopMargin;
        frame->topHeight = frame->fixedTopHeight;
    } else {
        frame->topMargin = frame->topHeight = 0;
    }

    frame->fixedLeftWidth = q->elementSize(frame->prefix % QLatin1String("left")).width();

    if (q->hasElement(frame->prefix % QLatin1String("hint-left-margin"))) {
        frame->fixedLeftMargin = q->elementSize(frame->prefix % QLatin1String("hint-left-margin")).width();
    } else {
        frame->fixedLeftMargin = frame->fixedLeftWidth;
    }

    if (frame->enabledBorders & FrameSvg::LeftBorder) {
        frame->leftMargin = frame->fixedLeftMargin;
        frame->leftWidth = frame->fixedLeftWidth;
    } else {
        frame->leftMargin = frame->leftWidth = 0;
    }

    frame->fixedRightWidth = q->elementSize(frame->prefix % QLatin1String("right")).width();

    if (q->hasElement(frame->prefix % QLatin1String("hint-right-margin"))) {
        frame->fixedRightMargin = q->elementSize(frame->prefix % QLatin1String("hint-right-margin")).width();
    } else {
        frame->fixedRightMargin = frame->fixedRightWidth;
    }

    if (frame->enabledBorders & FrameSvg::RightBorder) {
        frame->rightMargin = frame->fixedRightMargin;
        frame->rightWidth = frame->fixedRightWidth;
    } else {
        frame->rightMargin = frame->rightWidth = 0;
    }

    frame->fixedBottomHeight = q->elementSize(frame->prefix % QLatin1String("bottom")).height();

    if (q->hasElement(frame->prefix % QLatin1String("hint-bottom-margin"))) {
        frame->fixedBottomMargin = q->elementSize(frame->prefix % QLatin1String("hint-bottom-margin")).height();
    } else {
        frame->fixedBottomMargin = frame->fixedBottomHeight;
    }

    if (frame->enabledBorders & FrameSvg::BottomBorder) {
        frame->bottomMargin = frame->fixedBottomMargin;
        frame->bottomHeight = frame->fixedBottomHeight;
    } else {
        frame->bottomMargin = frame->bottomHeight = 0;
    }

    frame->composeOverBorder = (q->hasElement(frame->prefix % QLatin1String("hint-compose-over-border")) &&
                                q->hasElement(QLatin1String("mask-") % frame->prefix % QLatin1String("center")));

    //since it's rectangular, topWidth and bottomWidth must be the same
    //the ones that don't have a frame->prefix is for retrocompatibility
    frame->tileCenter = (q->hasElement(QStringLiteral("hint-tile-center")) || q->hasElement(frame->prefix % QLatin1String("hint-tile-center")));
    frame->noBorderPadding = (q->hasElement(QStringLiteral("hint-no-border-padding")) || q->hasElement(frame->prefix % QLatin1String("hint-no-border-padding")));
    frame->stretchBorders = (q->hasElement(QStringLiteral("hint-stretch-borders")) || q->hasElement(frame->prefix % QLatin1String("hint-stretch-borders")));
    q->resize(s);
}

void FrameSvgPrivate::updateNeeded()
{
    q->setElementPrefix(requestedPrefix);
    //frame not created yet?
    if (!frame) {
        return;
    }
    q->clearCache();
    updateSizes(frame);
}

void FrameSvgPrivate::updateAndSignalSizes()
{
    //frame not created yet?
    if (!frame) {
        return;
    }
    updateSizes(frame);
    emit q->repaintNeeded();
}

QSizeF FrameSvgPrivate::frameSize(FrameData *frame) const
{
    if (!frame) {
        return QSizeF();
    }

    if (!frame->frameSize.isValid()) {
        updateSizes(frame);
        frame->frameSize = q->size();
    }

    return frame->frameSize;
}

QString FrameSvg::actualPrefix() const
{
    return d->prefix;
}

bool FrameSvg::isRepaintBlocked() const
{
    return d->repaintBlocked;
}

void FrameSvg::setRepaintBlocked(bool blocked)
{
    d->repaintBlocked = blocked;

    if (!blocked) {
        d->updateFrameData();
    }
}

} // Plasma namespace
