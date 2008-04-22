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
    }

    void generateBackground();
    void updateSizes();
    void updateAndSignalSizes();

    Location location;
    QString prefix;

    PanelSvg *q;

    bool cacheAll : 1;

    QHash<QString, PanelData*> panels;
};

PanelSvg::PanelSvg(const QString& imagePath, QObject* parent)
    : Svg(imagePath, parent),
      d(new Private(this))
{
    connect(this, SIGNAL(repaintNeeded()), this, SLOT(updateSizes()));

    d->panels.insert(QString(), new PanelData());
    d->updateSizes();
    d->panels[QString()]->panelSize = size();
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

    qDeleteAll(d->panels);

    setImagePath(path);
}

void PanelSvg::setEnabledBorders(const EnabledBorders borders)
{
    if (borders == d->panels[d->prefix]->enabledBorders) {
        return;
    }

    d->panels[d->prefix]->enabledBorders = borders;
    d->updateSizes();
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
        return QString();
    }
    return d->prefix.left(d->prefix.size() - 1);
}

void PanelSvg::resize(const QSizeF& size)
{
    if (!size.isValid() || size.width() < 1 || size.height() < 1 || size == d->panels[d->prefix]->panelSize) {
        return;
    }

    d->panels[d->prefix]->panelSize = size;
    d->updateSizes();
}

void PanelSvg::resize(qreal width, qreal height)
{
    resize(QSize(width, height));
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

QBitmap PanelSvg::mask() const
{
    if (!d->panels[d->prefix]->cachedBackground) {
        d->generateBackground();
    }

    return d->panels[d->prefix]->cachedBackground->alphaChannel().createMaskFromColor(Qt::black);
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
    qDeleteAll(d->panels);

    if (!d->panels.contains(d->prefix)) {
        d->panels.insert(d->prefix, new PanelData());
    }
}

void PanelSvg::paint(QPainter* painter, const QRectF& rect, const QPointF& pos)
{
    if (!d->panels[d->prefix]->cachedBackground) {
        d->generateBackground();
    }

    //FIXME: this is redundant with generatebackground for now
    bool origined = d->panels[d->prefix]->contentAtOrigin;
    const int topOffset = origined ? 0 - d->panels[d->prefix]->topHeight : 0;
    const int leftOffset = origined ? 0 - d->panels[d->prefix]->leftWidth : 0;

    painter->drawPixmap(rect, *d->panels[d->prefix]->cachedBackground, rect.translated(-pos.x()-leftOffset,-pos.y()-topOffset));
}

void PanelSvg::Private::generateBackground()
{
    PanelData *panel = panels[prefix];

    bool origined = panel->contentAtOrigin;
    const int topWidth = q->elementSize(prefix + "top").width();
    const int leftHeight = q->elementSize(prefix + "left").height();
    const int topOffset = origined ? 0 - panel->topHeight : 0;
    const int leftOffset = origined ? 0 - panel->leftWidth : 0;

    if (!panel->cachedBackground) {
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
                   q->Svg::paint(&centerPainter, QPoint(0, 0), prefix + "center");
               }

               p.drawTiledPixmap(QRect(contentLeft - panel->leftWidth, contentTop - panel->topHeight,
                                          contentWidth + panel->leftWidth*2, contentHeight + panel->topHeight*2), center);
           }
       } else {
           if (contentHeight > 0 && contentWidth > 0) {
               QSizeF scaledSize = QSizeF(panel->panelSize.width() -
                                (panel->leftWidth + panel->rightWidth) +
                                panel->panelSize.width()*(((qreal)(panel->leftWidth + panel->rightWidth)) / panel->panelSize.width()),
                                panel->panelSize.height() -
                                (panel->topHeight + panel->bottomHeight) +
                                panel->panelSize.height()*(((qreal)(panel->topHeight + panel->bottomHeight)) / panel->panelSize.height()));

               q->Svg::resize(scaledSize.width(), scaledSize.height());
               q->Svg::paint(&p, QRect(contentLeft - panel->leftWidth, contentTop - panel->topHeight,
                                              contentWidth + panel->leftWidth*2, contentHeight + panel->topHeight*2),
                                   prefix + "center");
               q->Svg::resize();
           }
       }

        // Corners
        if (panel->enabledBorders & TopBorder) {
            if (!origined) {
                contentTop = panel->topHeight;
                bottomOffset += panel->topHeight;
            }

            if (panel->enabledBorders & LeftBorder) {
                q->Svg::paint(&p, QRect(leftOffset, topOffset, panel->leftWidth, panel->topHeight), prefix + "topleft");

                if (!origined) {
                    contentLeft = panel->leftWidth;
                    rightOffset = contentWidth + panel->leftWidth;
                }
            }

            if (panel->enabledBorders & RightBorder) {
                q->Svg::paint(&p, QRect(rightOffset, topOffset, panel->rightWidth, panel->topHeight), prefix + "topright");
            }
        }

        if (panel->enabledBorders & BottomBorder) {
            if (panel->enabledBorders & LeftBorder) {
                q->Svg::paint(&p, QRect(leftOffset, bottomOffset, panel->leftWidth, panel->bottomHeight), prefix + "bottomleft");

                if (!origined) {
                    contentLeft = panel->leftWidth;
                    rightOffset = contentWidth + panel->leftWidth;
                }
            }

            if (panel->enabledBorders & RightBorder) {
                q->Svg::paint(&p, QRect(rightOffset, bottomOffset, panel->rightWidth, panel->bottomHeight), prefix + "bottomright");
            }
        }

        // Sides
        if (panel->stretchBorders) {
            if (panel->enabledBorders & LeftBorder) {
                q->Svg::paint(&p, QRect(leftOffset, contentTop, panel->leftWidth, contentHeight), prefix + "left");
            }

            if (panel->enabledBorders & RightBorder) {
                q->Svg::paint(&p, QRect(rightOffset, contentTop, panel->rightWidth, contentHeight), prefix + "right");
            }

            if (panel->enabledBorders & TopBorder) {
                q->Svg::paint(&p, QRect(contentLeft, topOffset, contentWidth, panel->topHeight), prefix + "top");
            }

            if (panel->enabledBorders & BottomBorder) {
                q->Svg::paint(&p, QRect(contentLeft, bottomOffset, contentWidth, panel->bottomHeight), prefix + "bottom");
            }
        } else {
            if (panel->enabledBorders & LeftBorder) {
                QPixmap left(panel->leftWidth, leftHeight);
                left.fill(Qt::transparent);

                {
                    QPainter sidePainter(&left);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    q->Svg::paint(&sidePainter, QPoint(0, 0), prefix + "left");
                }

                p.drawTiledPixmap(QRect(leftOffset, contentTop, panel->leftWidth, contentHeight), left);
            }

            if (panel->enabledBorders & RightBorder) {
                QPixmap right(panel->rightWidth, leftHeight);
                right.fill(Qt::transparent);

                {
                    QPainter sidePainter(&right);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    q->Svg::paint(&sidePainter, QPoint(0, 0), prefix + "right");
                }

                p.drawTiledPixmap(QRect(rightOffset, contentTop, panel->rightWidth, contentHeight), right);
            }

            if (panel->enabledBorders & TopBorder) {
                QPixmap top(topWidth, panel->topHeight);
                top.fill(Qt::transparent);

                {
                    QPainter sidePainter(&top);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    q->Svg::paint(&sidePainter, QPoint(0, 0), prefix + "top");
                }

                p.drawTiledPixmap(QRect(contentLeft, topOffset, contentWidth, panel->topHeight), top);
            }

            if (panel->enabledBorders & BottomBorder) {
                QPixmap bottom(topWidth, panel->bottomHeight);
                bottom.fill(Qt::transparent);

                {
                    QPainter sidePainter(&bottom);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    q->Svg::paint(&sidePainter, QPoint(0, 0), prefix + "bottom");
                }

                p.drawTiledPixmap(QRect(contentLeft, bottomOffset, contentWidth, panel->bottomHeight), bottom);
            }
        }

        // re-enable this once Qt's svg rendering is un-buggered
        //q->Svg::resize(contentWidth, contentHeight);
        //paint(&p, QRect(contentLeft, contentTop, contentWidth, contentHeight), "center");
   }
}

void PanelSvg::Private::updateSizes()
{
    PanelData *panel = panels[prefix];

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
    updateSizes();
    emit q->repaintNeeded();
}

} // Plasma namespace

#include "panelsvg.moc"
