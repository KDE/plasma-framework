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

#include "panelsvg.h"

#include <QPainter>
#include <QSize>
#include <QBitmap>

#include <KDebug>

namespace Plasma
{

class PanelData
{
public:
    PanelData()
      : enabledBorders(PanelSvg::AllBorders),
        cachedBackground(0),
        panelSize(-1,-1),
        contentAtOrigin(false)
    {
    }

    ~PanelData()
    {
        delete cachedBackground;
    }

    PanelSvg::EnabledBorders enabledBorders;
    QPixmap *cachedBackground;
    QSizeF panelSize;

    //measures
    int topHeight;
    int leftWidth;
    int rightWidth;
    int bottomHeight;

    //size of the svg where the size of the "center"
    //element is contentWidth x contentHeight
    bool noBorderPadding : 1;
    bool stretchBorders : 1;
    bool tileCenter : 1;
    bool contentAtOrigin : 1;
};

class PanelSvg::Private
{
public:
    Private(PanelSvg *psvg)
      : q(psvg),
        cacheAll(false)
    {
    }

    ~Private()
    {
        qDeleteAll(panels);
        panels.clear();
    }

    void generateBackground(PanelData *panel);
    void updateSizes();
    void updateAndSignalSizes();

    Location location;
    QString prefix;

    PanelSvg *q;

    bool cacheAll : 1;

    QHash<QString, PanelData*> panels;
};

PanelSvg::PanelSvg(QObject* parent)
    : Svg(parent),
      d(new Private(this))
{
    connect(this, SIGNAL(repaintNeeded()), this, SLOT(updateSizes()));
    d->panels.insert(QString(), new PanelData());
}

PanelSvg::~PanelSvg()
{
    delete d;
}

void PanelSvg::setImagePath(const QString& path)
{
    if (path == imagePath()) {
        return;
    }

    Svg::setImagePath(path);

    clearCache();
    d->updateAndSignalSizes();
}

void PanelSvg::setEnabledBorders(const EnabledBorders borders)
{
    if (borders == d->panels[d->prefix]->enabledBorders) {
        return;
    }

    d->panels[d->prefix]->enabledBorders = borders;
    d->updateAndSignalSizes();
}

PanelSvg::EnabledBorders PanelSvg::enabledBorders() const
{
    return d->panels[d->prefix]->enabledBorders;
}

void PanelSvg::setElementPrefix(Plasma::Location location)
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

bool PanelSvg::hasElementPrefix(const QString & prefix) const
{
    //for now it simply checks if a center element exists,
    //because it could make sense for certain themes to not have all the elements
    return hasElement(prefix + "-center");
}

void PanelSvg::setElementPrefix(const QString & prefix)
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

    if (oldPrefix == d->prefix && d->panels[oldPrefix]) {
        return;
    }

    if (!d->cacheAll) {
        delete d->panels[oldPrefix];
        d->panels.remove(oldPrefix);
    }

    if (!d->panels.contains(d->prefix)) {
        d->panels.insert(d->prefix, new PanelData());
    }

    d->location = Floating;
}

QString PanelSvg::prefix()
{
    if (d->prefix.isEmpty()) {
        return d->prefix;
    }

    return d->prefix.left(d->prefix.size() - 1);
}

void PanelSvg::resizePanel(const QSizeF& size)
{
    bool sizeValid = size.width() > 0 && size.height() > 0;
    if (!sizeValid || size == d->panels[d->prefix]->panelSize) {
        if (!sizeValid)
            kWarning() << "Invalid size" << size;
        return;
    }

    d->updateSizes();
    d->panels[d->prefix]->panelSize = size;
}

qreal PanelSvg::marginSize(const Plasma::MarginEdge edge) const
{
    if (d->panels[d->prefix]->noBorderPadding) {
        return .0;
    }

    switch (edge) {
    case Plasma::TopMargin:
        return d->panels[d->prefix]->topHeight;
    break;

    case Plasma::LeftMargin:
        return d->panels[d->prefix]->leftWidth;
    break;

    case Plasma::RightMargin:
        return d->panels[d->prefix]->rightWidth;
    break;

    //Plasma::BottomMargin
    default:
        return d->panels[d->prefix]->bottomHeight;
    break;
    }
}

void PanelSvg::getMargins(qreal &left, qreal &top, qreal &right, qreal &bottom) const
{
    top = marginSize(Plasma::TopMargin);
    left = marginSize(Plasma::LeftMargin);
    right = marginSize(Plasma::RightMargin);
    bottom = marginSize(Plasma::BottomMargin);
}

QBitmap PanelSvg::mask() const
{
    PanelData *panel = d->panels[d->prefix];
    if (!panel->cachedBackground) {
        d->generateBackground(panel);
        Q_ASSERT(panel->cachedBackground);
    }

    return panel->cachedBackground->alphaChannel().createMaskFromColor(Qt::black);
}

void PanelSvg::setCacheAllRenderedPanels(bool cache)
{
    if (d->cacheAll && !cache) {
        clearCache();
    }

    d->cacheAll = cache;
}

bool PanelSvg::cacheAllRenderedPanels() const
{
    return d->cacheAll;
}

void PanelSvg::clearCache()
{
    PanelData *panel = d->panels[d->prefix];
    if (panel) {
        // make a copy of the panel data to preserve settings,
        // but then reset the cached image
        panel = new PanelData(*panel);
        panel->cachedBackground = 0;
    } else {
        panel = new PanelData();
    }

    qDeleteAll(d->panels);
    d->panels.clear();

    d->panels[d->prefix] = panel;
}

void PanelSvg::paintPanel(QPainter* painter, const QRectF& rect, const QPointF& pos)
{
    PanelData *panel = d->panels[d->prefix];
    if (!panel->cachedBackground) {
        d->generateBackground(panel);
        Q_ASSERT(panel->cachedBackground);
    }

    //FIXME: this is redundant with generatebackground for now
    bool origined = panel->contentAtOrigin;
    const int topOffset = origined ? 0 - panel->topHeight : 0;
    const int leftOffset = origined ? 0 - panel->leftWidth : 0;

    painter->drawPixmap(rect, *(panel->cachedBackground), rect.translated(-pos.x()-leftOffset,-pos.y()-topOffset));
}

void PanelSvg::Private::generateBackground(PanelData *panel)
{
    bool origined = panel->contentAtOrigin;
    const int topWidth = q->elementSize(prefix + "top").width();
    const int leftHeight = q->elementSize(prefix + "left").height();
    const int topOffset = origined ? 0 - panel->topHeight : 0;
    const int leftOffset = origined ? 0 - panel->leftWidth : 0;

    if (panel->cachedBackground) {
        return;
    }
    if (!panel->panelSize.isValid()) {
        kWarning() << "Invalid panel size" << panel->panelSize;
        panel->cachedBackground = new QPixmap();
        return;
    }

    const int contentWidth = panel->panelSize.width() - panel->leftWidth  - panel->rightWidth;
    const int contentHeight = panel->panelSize.height() - panel->topHeight  - panel->bottomHeight;
    int contentTop = 0;
    int contentLeft = 0;
    int rightOffset = contentWidth;
    int bottomOffset = contentHeight;

    panel->cachedBackground = new QPixmap(panel->leftWidth + contentWidth + panel->rightWidth,
                                          panel->topHeight + contentHeight + panel->bottomHeight);
    panel->cachedBackground->fill(Qt::transparent);
    QPainter p(panel->cachedBackground);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    if (origined) {
        p.translate(panel->leftWidth, panel->topHeight);
    }

    //FIXME: This is a hack to fix a drawing problems with svg files where a thin transparent border is drawn around the svg image.
    //       the transparent border around the svg seems to vary in size depending on the size of the svg and as a result increasing the
    //       svg image by 2 all around didn't resolve the issue. For now it resizes based on the border size.

    //if we must stretch the center or the borders we compute how much we will have to stretch
    //the svg to get the desired element sizes
    QSizeF  scaledContentSize(0,0);
    if (q->elementSize(prefix + "center").width() > 0 &&
        q->elementSize(prefix + "center").height() > 0 &&
        (!panel->tileCenter || panel->stretchBorders)) {
        scaledContentSize = QSizeF(contentWidth * ((qreal)q->size().width() / (qreal)q->elementSize(prefix + "center").width()),
                                   contentHeight * ((qreal)q->size().height() / (qreal)q->elementSize(prefix + "center").height()));
    }

    //CENTER
    if (panel->tileCenter) {
        if (contentHeight > 0 && contentWidth > 0) {
            int centerTileHeight;
            int centerTileWidth;
            centerTileHeight = q->elementSize(prefix + "center").height();
            centerTileWidth = q->elementSize(prefix + "center").width();
            QPixmap center(centerTileWidth, centerTileHeight);
            center.fill(Qt::transparent);

            {
                QPainter centerPainter(&center);
                centerPainter.setCompositionMode(QPainter::CompositionMode_Source);
                q->paint(&centerPainter, QPoint(0, 0), prefix + "center");
            }

            p.drawTiledPixmap(QRect(panel->leftWidth, panel->topHeight,
                                    contentWidth, contentHeight), center);
        }
    } else {
        if (contentHeight > 0 && contentWidth > 0) {

            q->resize(scaledContentSize);
            
            q->paint(&p, QRect(panel->leftWidth, panel->topHeight,
                               contentWidth, contentHeight),
                               prefix + "center");
            q->resize();
        }
    }

    // Corners
    if (panel->enabledBorders & TopBorder) {
        if (!origined) {
            contentTop = panel->topHeight;
            bottomOffset += panel->topHeight;
        }

        if (panel->enabledBorders & LeftBorder) {
            q->paint(&p, QRect(leftOffset, topOffset, panel->leftWidth, panel->topHeight), prefix + "topleft");

            if (!origined) {
                contentLeft = panel->leftWidth;
                rightOffset = contentWidth + panel->leftWidth;
            }
        }

        if (panel->enabledBorders & RightBorder) {
            q->paint(&p, QRect(rightOffset, topOffset, panel->rightWidth, panel->topHeight), prefix + "topright");
        }
    }

    if (panel->enabledBorders & BottomBorder) {
        if (panel->enabledBorders & LeftBorder) {
            q->paint(&p, QRect(leftOffset, bottomOffset, panel->leftWidth, panel->bottomHeight), prefix + "bottomleft");

            if (!origined) {
                contentLeft = panel->leftWidth;
                rightOffset = contentWidth + panel->leftWidth;
            }
        }

        if (panel->enabledBorders & RightBorder) {
            q->paint(&p, QRect(rightOffset, bottomOffset, panel->rightWidth, panel->bottomHeight), prefix + "bottomright");
        }
    }

    // Sides
    if (panel->stretchBorders) {
        if (panel->enabledBorders & LeftBorder || panel->enabledBorders & RightBorder) {
            q->resize(q->size().width(), scaledContentSize.height());

            if (panel->enabledBorders & LeftBorder) {
                q->paint(&p, QRect(leftOffset, contentTop, panel->leftWidth, contentHeight), prefix + "left");
            }

            if (panel->enabledBorders & RightBorder) {
                q->paint(&p, QRect(rightOffset, contentTop, panel->rightWidth, contentHeight), prefix + "right");
            }

            q->resize();
        }

        if (panel->enabledBorders & TopBorder || panel->enabledBorders & BottomBorder) {
            q->resize(scaledContentSize.width(), q->size().height());

            if (panel->enabledBorders & TopBorder) {
                q->paint(&p, QRect(contentLeft, topOffset, contentWidth, panel->topHeight), prefix + "top");
            }

            if (panel->enabledBorders & BottomBorder) {
                q->paint(&p, QRect(contentLeft, bottomOffset, contentWidth, panel->bottomHeight), prefix + "bottom");
            }

            q->resize();
        }
    } else {
        if (panel->enabledBorders & LeftBorder) {
            QPixmap left(panel->leftWidth, leftHeight);
            left.fill(Qt::transparent);

            {
                QPainter sidePainter(&left);
                sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                q->paint(&sidePainter, QPoint(0, 0), prefix + "left");
            }

            p.drawTiledPixmap(QRect(leftOffset, contentTop, panel->leftWidth, contentHeight), left);
        }

        if (panel->enabledBorders & RightBorder) {
            QPixmap right(panel->rightWidth, leftHeight);
            right.fill(Qt::transparent);

            {
                QPainter sidePainter(&right);
                sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                q->paint(&sidePainter, QPoint(0, 0), prefix + "right");
            }

            p.drawTiledPixmap(QRect(rightOffset, contentTop, panel->rightWidth, contentHeight), right);
        }

        if (panel->enabledBorders & TopBorder) {
            QPixmap top(topWidth, panel->topHeight);
            top.fill(Qt::transparent);

            {
                QPainter sidePainter(&top);
                sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                q->paint(&sidePainter, QPoint(0, 0), prefix + "top");
            }

            p.drawTiledPixmap(QRect(contentLeft, topOffset, contentWidth, panel->topHeight), top);
        }

        if (panel->enabledBorders & BottomBorder) {
            QPixmap bottom(topWidth, panel->bottomHeight);
            bottom.fill(Qt::transparent);

            {
                QPainter sidePainter(&bottom);
                sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                q->paint(&sidePainter, QPoint(0, 0), prefix + "bottom");
            }

            p.drawTiledPixmap(QRect(contentLeft, bottomOffset, contentWidth, panel->bottomHeight), bottom);
        }
    }

    // re-enable this once Qt's svg rendering is un-buggered
    //q->resize(contentWidth, contentHeight);
    //paint(&p, QRect(contentLeft, contentTop, contentWidth, contentHeight), "center");
}

void PanelSvg::Private::updateSizes()
{
    //kDebug() << "!!!!!!!!!!!!!!!!!!!!!! updating sizes" << prefix;
    PanelData *panel = panels[prefix];
    Q_ASSERT(panel);

    delete panel->cachedBackground;
    panel->cachedBackground = 0;

    q->Svg::resize();
    if (panel->enabledBorders & TopBorder) {
        panel->topHeight = q->elementSize(prefix + "top").height();
    } else {
        panel->topHeight = 0;
    }

    if (panel->enabledBorders & LeftBorder) {
        panel->leftWidth = q->elementSize(prefix + "left").width();
    } else {
        panel->leftWidth = 0;
    }

    if (panel->enabledBorders & RightBorder) {
        panel->rightWidth = q->elementSize(prefix + "right").width();
    } else {
        panel->rightWidth = 0;
    }

    if (panel->enabledBorders & BottomBorder) {
        panel->bottomHeight = q->elementSize(prefix + "bottom").height();
    } else {
        panel->bottomHeight = 0;
    }

    //since it's rectangular, topWidth and bottomWidth must be the same
    panel->tileCenter = q->hasElement("hint-tile-center");
    panel->noBorderPadding = q->hasElement("hint-no-border-padding");
    panel->stretchBorders = q->hasElement("hint-stretch-borders");
}

void PanelSvg::Private::updateAndSignalSizes()
{
    // updateSizes(); <-- this gets called when repaintNeeded is emitted
    emit q->repaintNeeded();
}

} // Plasma namespace

#include "panelsvg.moc"
