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

#include "svgpanel.h"

#include <QPainter>
#include <QSize>
#include <QBitmap>

#include <KDebug>

namespace Plasma
{

class SvgPanel::Private
{
public:
    Private()
      : bFlags(DrawAllBorders|ContentAtOrigin),
        cachedBackground(0), pos(0,0)
    {
    }

    ~Private()
    {
        delete cachedBackground;
    }

    void generateBackground();

    BorderFlags bFlags;
    QPixmap *cachedBackground;
    Svg *background;
    QSizeF panelSize;
    QPointF pos;

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
};

SvgPanel::SvgPanel(const QString& imagePath, QObject* parent)
    : QObject(parent),
      d(new Private)
{
    d->background = new Svg(imagePath, this);
    connect(d->background, SIGNAL(repaintNeeded()), this, SLOT(updateSizes()));

    updateSizes();
    d->panelSize = d->background->size();
}

SvgPanel::~SvgPanel()
{
    delete d;
}

void SvgPanel::setFile(const QString& imagePath)
{
    if (imagePath == d->background->file()) {
        return;
    }

    d->background->setFile(imagePath);
    updateSizes();
}

QString SvgPanel::file() const
{
   return d->background->file();
}

void SvgPanel::setBorderFlags(const BorderFlags flags)
{
    if (flags == d->bFlags) {
        return;
    }

    d->bFlags = flags;
    updateSizes();
}

SvgPanel::BorderFlags SvgPanel::borderFlags() const
{
    return d->bFlags;
}

void SvgPanel::setPos(const QPointF& pos)
{
    d->pos = pos;
}

QPointF SvgPanel::pos() const
{
    return d->pos;
}


void SvgPanel::resize(const QSizeF& size)
{
    if (!size.isValid() || size.width() < 1 || size.height() < 1 || size == d->panelSize) {
        return;
    }

    d->panelSize = size;
    updateSizes();
}

qreal SvgPanel::marginSize(const Plasma::MarginEdge edge) const
{
    if (d->noBorderPadding) {
        return .0;
    }

    switch (edge) {
    case Plasma::TopMargin:
        return d->topHeight;
    break;

    case Plasma::LeftMargin:
        return d->leftWidth;
    break;

    case Plasma::RightMargin:
        return d->rightWidth;
    break;

    //Plasma::BottomMargin
    default:
        return d->bottomHeight;
    break;
    }
}

QBitmap SvgPanel::mask() const
{
    if (!d->cachedBackground) {
        d->generateBackground();
    }

    return d->cachedBackground->alphaChannel().createMaskFromColor(Qt::black);
}

void SvgPanel::Private::generateBackground()
{
    bool origined = bFlags & ContentAtOrigin;
    const int topWidth = background->elementSize("top").width();
    const int leftHeight = background->elementSize("left").height();
    const int topOffset = origined ? 0 - topHeight : 0;
    const int leftOffset = origined ? 0 - leftWidth : 0;

    if (!cachedBackground) {
        const int contentWidth = panelSize.width() - leftWidth  - rightWidth;
        const int contentHeight = panelSize.height() - topHeight  - bottomHeight;
        int contentTop = 0;
        int contentLeft = 0;
        int rightOffset = contentWidth;
        int bottomOffset = contentHeight;

        delete cachedBackground;
        cachedBackground = new QPixmap(leftWidth + contentWidth + rightWidth,
                                          topHeight + contentHeight + bottomHeight);
        cachedBackground->fill(Qt::transparent);
        QPainter p(cachedBackground);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.setRenderHint(QPainter::SmoothPixmapTransform);

        if (origined) {
            p.translate(leftWidth, topHeight);
        }

        //FIXME: This is a hack to fix a drawing problems with svg files where a thin transparent border is drawn around the svg image.
        //       the transparent border around the svg seems to vary in size depending on the size of the svg and as a result increasing the
        //       svg image by 2 all around didn't resolve the issue. For now it resizes based on the border size.


       //CENTER
       if (tileCenter) { 
           if (contentHeight > 0 && contentWidth > 0) {
               int centerTileHeight;
               int centerTileWidth;
               centerTileHeight = background->elementSize("center").height();
               centerTileWidth = background->elementSize("center").width();
               QPixmap center(centerTileWidth, centerTileHeight);
               center.fill(Qt::transparent);

               {
                   QPainter centerPainter(&center);
                   centerPainter.setCompositionMode(QPainter::CompositionMode_Source);
                   background->paint(&centerPainter, QPoint(0, 0), "center");
               }

               p.drawTiledPixmap(QRect(contentLeft - leftWidth, contentTop - topHeight,
                                          contentWidth + leftWidth*2, contentHeight + topHeight*2), center);
           }
       } else {
           if (contentHeight > 0 && contentWidth > 0) {
               QSizeF scaledSize = QSizeF(panelSize.width() -
                                (leftWidth + rightWidth) +
                                panelSize.width()*(((qreal)(leftWidth + rightWidth)) / panelSize.width()),
                                panelSize.height() -
                                (topHeight + bottomHeight) +
                                panelSize.height()*(((qreal)(topHeight + bottomHeight)) / panelSize.height()));
    
               background->resize(scaledSize.width(), scaledSize.height());
               background->paint(&p, QRect(contentLeft - leftWidth, contentTop - topHeight,
                                              contentWidth + leftWidth*2, contentHeight + topHeight*2),
                                   "center");
               background->resize();
           }
       }


        // Corners
        if (bFlags & DrawTopBorder) {
            if (!origined) {
                contentTop = topHeight;
                bottomOffset += topHeight;
            }

            if (bFlags & DrawLeftBorder) {
                background->paint(&p, QRect(leftOffset, topOffset, leftWidth, topHeight), "topleft");

                if (!origined) {
                    contentLeft = leftWidth;
                    rightOffset = contentWidth + leftWidth;
                }
            }

            if (bFlags & DrawRightBorder) {
                background->paint(&p, QRect(rightOffset, topOffset, rightWidth, topHeight), "topright");
            }
        }

        if (bFlags & DrawBottomBorder) {
            if (bFlags & DrawLeftBorder) {
                background->paint(&p, QRect(leftOffset, bottomOffset, leftWidth, bottomHeight), "bottomleft");

                if (!origined) {
                    contentLeft = leftWidth;
                    rightOffset = contentWidth + leftWidth;
                }
            }

            if (bFlags & DrawRightBorder) {
                background->paint(&p, QRect(rightOffset, bottomOffset, rightWidth, bottomHeight), "bottomright");
            }
        }

        // Sides
        if (stretchBorders) {
            if (bFlags & DrawLeftBorder) {
                background->paint(&p, QRect(leftOffset, contentTop, leftWidth, contentHeight), "left");
            }

            if (bFlags & DrawRightBorder) {
                background->paint(&p, QRect(rightOffset, contentTop, rightWidth, contentHeight), "right");
            }

            if (bFlags & DrawTopBorder) {
                background->paint(&p, QRect(contentLeft, topOffset, contentWidth, topHeight), "top");
            }

            if (bFlags & DrawBottomBorder) {
                background->paint(&p, QRect(contentLeft, bottomOffset, contentWidth, bottomHeight), "bottom");
            }
        } else {
            if (bFlags & DrawLeftBorder) {
                QPixmap left(leftWidth, leftHeight);
                left.fill(Qt::transparent);

                {
                    QPainter sidePainter(&left);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    background->paint(&sidePainter, QPoint(0, 0), "left");
                }

                p.drawTiledPixmap(QRect(leftOffset, contentTop, leftWidth, contentHeight), left);
            }

            if (bFlags & DrawRightBorder) {
                QPixmap right(rightWidth, leftHeight);
                right.fill(Qt::transparent);

                {
                    QPainter sidePainter(&right);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    background->paint(&sidePainter, QPoint(0, 0), "right");
                }

                p.drawTiledPixmap(QRect(rightOffset, contentTop, rightWidth, contentHeight), right);
            }

            if (bFlags & DrawTopBorder) {
                QPixmap top(topWidth, topHeight);
                top.fill(Qt::transparent);

                {
                    QPainter sidePainter(&top);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    background->paint(&sidePainter, QPoint(0, 0), "top");
                }

                p.drawTiledPixmap(QRect(contentLeft, topOffset, contentWidth, topHeight), top);
            }

            if (bFlags & DrawBottomBorder) {
                QPixmap bottom(topWidth, bottomHeight);
                bottom.fill(Qt::transparent);

                {
                    QPainter sidePainter(&bottom);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    background->paint(&sidePainter, QPoint(0, 0), "bottom");
                }

                p.drawTiledPixmap(QRect(contentLeft, bottomOffset, contentWidth, bottomHeight), bottom);
            }
        }

        // re-enable this once Qt's svg rendering is un-buggered
        //resize(contentWidth, contentHeight);
        //paint(&p, QRect(contentLeft, contentTop, contentWidth, contentHeight), "center");
   }
}

void SvgPanel::paint(QPainter* painter, const QRectF& rect)
{
    if (!d->cachedBackground) {
        d->generateBackground();
    }

    //FIXME: this is redundant with generatebackground for now
    bool origined = d->bFlags & ContentAtOrigin;
    const int topWidth = d->background->elementSize("top").width();
    const int leftHeight = d->background->elementSize("left").height();
    const int topOffset = origined ? 0 - d->topHeight : 0;
    const int leftOffset = origined ? 0 - d->leftWidth : 0;

    painter->drawPixmap(rect, *d->cachedBackground, rect.translated(-d->pos.x()-leftOffset,-d->pos.y()-topOffset));
}

void SvgPanel::updateSizes()
{
    delete d->cachedBackground;
    d->cachedBackground = 0;
    d->background->resize();
    if (d->bFlags & DrawTopBorder) {
        d->topHeight = d->background->elementSize("top").height();
    } else {
        d->topHeight = 0;
    }

    if (d->bFlags & DrawLeftBorder) {
        d->leftWidth = d->background->elementSize("left").width();
    } else {
        d->leftWidth = 0;
    }

    if (d->bFlags & DrawRightBorder) {
        d->rightWidth = d->background->elementSize("right").width();
    } else {
        d->rightWidth = 0;
    }

   if (d->bFlags & DrawBottomBorder) {
       d->bottomHeight = d->background->elementSize("bottom").height();
   } else {
       d->bottomHeight = 0;
   }

   //since it's rectangular, topWidth and bottomWidth must be the same
   d->tileCenter = d->background->elementExists("hint-tile-center");
   d->noBorderPadding = d->background->elementExists("hint-no-border-padding");
   d->stretchBorders = d->background->elementExists("hint-stretch-borders");
   emit repaintNeeded();
}

} // Plasma namespace

#include "svgpanel.moc"
