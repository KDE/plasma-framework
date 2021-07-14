/*
    SPDX-FileCopyrightText: 2008 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2009 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_FRAMESVG_P_H
#define PLASMA_FRAMESVG_P_H

#include <QCache>
#include <QHash>
#include <QStringBuilder>

#include <QDebug>

#include <Plasma/Theme>

#include "svg_p.h"

namespace Plasma
{
class FrameData
{
public:
    FrameData(FrameSvg *svg, const QString &p)
        : imagePath(svg->imagePath())
        , prefix(p)
        , enabledBorders(FrameSvg::AllBorders)
        , frameSize(-1, -1)
        , topHeight(0)
        , leftWidth(0)
        , rightWidth(0)
        , bottomHeight(0)
        , topMargin(0)
        , leftMargin(0)
        , rightMargin(0)
        , bottomMargin(0)
        , noBorderPadding(false)
        , stretchBorders(false)
        , tileCenter(false)
        , composeOverBorder(false)
        , theme(nullptr)
    {
    }

    FrameData(const FrameData &other, FrameSvg *svg)
        : imagePath(other.imagePath)
        , prefix(other.prefix)
        , enabledBorders(other.enabledBorders)
        , cachedMasks(MAX_CACHED_MASKS)
        , frameSize(other.frameSize)
        , topHeight(0)
        , leftWidth(0)
        , rightWidth(0)
        , bottomHeight(0)
        , topMargin(0)
        , leftMargin(0)
        , rightMargin(0)
        , bottomMargin(0)
        , devicePixelRatio(svg->devicePixelRatio())
        , noBorderPadding(false)
        , stretchBorders(false)
        , tileCenter(false)
        , composeOverBorder(false)
        , theme(nullptr)
    {
    }

    ~FrameData();

    QString imagePath;
    QString prefix;
    QString requestedPrefix;
    FrameSvg::EnabledBorders enabledBorders;
    QPixmap cachedBackground;
    QCache<uint, QRegion> cachedMasks;
    static const int MAX_CACHED_MASKS = 10;
    uint lastModified = 0;

    QSize frameSize;
    uint cacheId;

    // measures
    int topHeight;
    int leftWidth;
    int rightWidth;
    int bottomHeight;

    // margins, are equal to the measures by default
    int topMargin;
    int leftMargin;
    int rightMargin;
    int bottomMargin;

    // measures
    int fixedTopHeight;
    int fixedLeftWidth;
    int fixedRightWidth;
    int fixedBottomHeight;

    // margins, are equal to the measures by default
    int fixedTopMargin;
    int fixedLeftMargin;
    int fixedRightMargin;
    int fixedBottomMargin;

    // margins, we only have the hint for insets
    int insetTopMargin;
    int insetLeftMargin;
    int insetRightMargin;
    int insetBottomMargin;

    qreal devicePixelRatio;

    // size of the svg where the size of the "center"
    // element is contentWidth x contentHeight
    bool noBorderPadding : 1;
    bool stretchBorders : 1;
    bool tileCenter : 1;
    bool composeOverBorder : 1;

    Plasma::ThemePrivate *theme;
};

class FrameSvgPrivate
{
public:
    FrameSvgPrivate(FrameSvg *psvg)
        : q(psvg)
        , overlayPos(0, 0)
        , enabledBorders(FrameSvg::AllBorders)
        , cacheAll(false)
        , repaintBlocked(false)
    {
    }

    ~FrameSvgPrivate();

    QPixmap alphaMask();

    enum UpdateType {
        UpdateFrame,
        UpdateFrameAndMargins,
    };

    void generateBackground(const QSharedPointer<FrameData> &frame);
    void generateFrameBackground(const QSharedPointer<FrameData> &);
    SvgPrivate::CacheId cacheId(FrameData *frame, const QString &prefixToUse) const;
    void cacheFrame(const QString &prefixToSave, const QPixmap &background, const QPixmap &overlay);
    void updateSizes(FrameData *frame) const;
    void updateSizes(const QSharedPointer<FrameData> &frame) const
    {
        return updateSizes(frame.data());
    }
    void updateNeeded();
    void updateAndSignalSizes();
    QSizeF frameSize(const QSharedPointer<FrameData> &frame) const
    {
        return frameSize(frame.data());
    }
    QSizeF frameSize(FrameData *frame) const;
    void paintBorder(QPainter &p,
                     const QSharedPointer<FrameData> &frame,
                     Plasma::FrameSvg::EnabledBorders border,
                     const QSize &originalSize,
                     const QRect &output) const;
    void paintCorner(QPainter &p, const QSharedPointer<FrameData> &frame, Plasma::FrameSvg::EnabledBorders border, const QRect &output) const;
    void paintCenter(QPainter &p, const QSharedPointer<FrameData> &frame, const QRect &contentRect, const QSize &fullSize);
    QRect contentGeometry(const QSharedPointer<FrameData> &frame, const QSize &size) const;
    void updateFrameData(uint lastModified, UpdateType updateType = UpdateFrameAndMargins);
    QSharedPointer<FrameData> lookupOrCreateMaskFrame(const QSharedPointer<FrameData> &frame, const QString &maskPrefix, const QString &maskRequestedPrefix);

    Types::Location location = Types::Floating;
    QString prefix;
    // sometimes the prefix we requested is not available, so prefix will be empty
    // keep track of the requested one anyways, we'll try again when the theme changes
    QString requestedPrefix;

    FrameSvg *const q;

    QPoint overlayPos;

    QSharedPointer<FrameData> frame;
    QSharedPointer<FrameData> maskFrame;

    // those can differ from frame->enabledBorders if we are in a transition
    FrameSvg::EnabledBorders enabledBorders;
    // this can differ from frame->frameSize if we are in a transition
    QSize pendingFrameSize;

    static QHash<ThemePrivate *, QHash<uint, QWeakPointer<FrameData>>> s_sharedFrames;

    bool cacheAll : 1;
    bool repaintBlocked : 1;
};

}

#endif
