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

#include <KDebug>

namespace Plasma
{

class SvgPanel::Private
{
public:
    Private()
      : bFlags(DrawAllBorders|ContentAtOrigin),
        cachedBackground(0)
    {
    }

    ~Private()
    {
        delete cachedBackground;
    }

    BorderFlags bFlags;
    QPixmap *cachedBackground;
    Svg *background;
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

    delete d->cachedBackground;
    d->cachedBackground = 0;
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

    delete d->cachedBackground;
    d->cachedBackground = 0;
    d->bFlags = flags;
    updateSizes();
}

SvgPanel::BorderFlags SvgPanel::borderFlags() const
{
    return d->bFlags;
}

void SvgPanel::resize(const QSizeF& size)
{
    if (!size.isValid() || size.width() < 1 || size.height() < 1 || size == d->panelSize) {
        return;
    }

    delete d->cachedBackground;
    d->cachedBackground = 0;
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

void SvgPanel::paint(QPainter* painter, const QRectF& rect)
{
    bool origined = d->bFlags & ContentAtOrigin;
    const int topWidth = d->background->elementSize("top").width();
    const int leftHeight = d->background->elementSize("left").height();
    const int topOffset = origined ? 0 - d->topHeight : 0;
    const int leftOffset = origined ? 0 - d->leftWidth : 0;

    if (!d->cachedBackground) {
        const int contentWidth = d->panelSize.width() - d->leftWidth  - d->rightWidth;
        const int contentHeight = d->panelSize.height() - d->topHeight  - d->bottomHeight;
        int contentTop = 0;
        int contentLeft = 0;
        int rightOffset = contentWidth;
        int bottomOffset = contentHeight;

        delete d->cachedBackground;
        d->cachedBackground = new QPixmap(d->leftWidth + contentWidth + d->rightWidth,
                                          d->topHeight + contentHeight + d->bottomHeight);
        d->cachedBackground->fill(Qt::transparent);
        QPainter p(d->cachedBackground);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.setRenderHint(QPainter::SmoothPixmapTransform);

        if (origined) {
            p.translate(d->leftWidth, d->topHeight);
        }

        //FIXME: This is a hack to fix a drawing problems with svg files where a thin transparent border is drawn around the svg image.
        //       the transparent border around the svg seems to vary in size depending on the size of the svg and as a result increasing the
        //       svg image by 2 all around didn't resolve the issue. For now it resizes based on the border size.


       //CENTER
       if (contentHeight > 0 && contentWidth > 0) {
           QSizeF scaledSize = QSizeF(d->panelSize.width() -
                            (d->leftWidth + d->rightWidth) +
                            d->panelSize.width()*(((qreal)(d->leftWidth + d->rightWidth)) / d->panelSize.width()),
                            d->panelSize.height() -
                            (d->topHeight + d->bottomHeight) +
                            d->panelSize.height()*(((qreal)(d->topHeight + d->bottomHeight)) / d->panelSize.height()));

           d->background->resize(scaledSize.width(), scaledSize.height());
           d->background->paint(&p, QRect(contentLeft - d->leftWidth, contentTop - d->topHeight,
                                          contentWidth + d->leftWidth*2, contentHeight + d->topHeight*2),
                               "center");
           d->background->resize();
       }

        // Corners
        if (d->bFlags & DrawTopBorder) {
            if (!origined) {
                contentTop = d->topHeight;
                bottomOffset += d->topHeight;
            }

            if (d->bFlags & DrawLeftBorder) {
                d->background->paint(&p, QRect(leftOffset, topOffset, d->leftWidth, d->topHeight), "topleft");

                if (!origined) {
                    contentLeft = d->leftWidth;
                    rightOffset = contentWidth + d->leftWidth;
                }
            }

            if (d->bFlags & DrawRightBorder) {
                d->background->paint(&p, QRect(rightOffset, topOffset, d->rightWidth, d->topHeight), "topright");
            }
        }

        if (d->bFlags & DrawBottomBorder) {
            if (d->bFlags & DrawLeftBorder) {
                d->background->paint(&p, QRect(leftOffset, bottomOffset, d->leftWidth, d->bottomHeight), "bottomleft");

                if (!origined) {
                    contentLeft = d->leftWidth;
                    rightOffset = contentWidth + d->leftWidth;
                }
            }

            if (d->bFlags & DrawRightBorder) {
                d->background->paint(&p, QRect(rightOffset, bottomOffset, d->rightWidth, d->bottomHeight), "bottomright");
            }
        }

        // Sides
        if (d->stretchBorders) {
            if (d->bFlags & DrawLeftBorder) {
                d->background->paint(&p, QRect(leftOffset, contentTop, d->leftWidth, contentHeight), "left");
            }

            if (d->bFlags & DrawRightBorder) {
                d->background->paint(&p, QRect(rightOffset, contentTop, d->rightWidth, contentHeight), "right");
            }

            if (d->bFlags & DrawTopBorder) {
                d->background->paint(&p, QRect(contentLeft, topOffset, contentWidth, d->topHeight), "top");
            }

            if (d->bFlags & DrawBottomBorder) {
                d->background->paint(&p, QRect(contentLeft, bottomOffset, contentWidth, d->bottomHeight), "bottom");
            }
        } else {
            if (d->bFlags & DrawLeftBorder) {
                QPixmap left(d->leftWidth, leftHeight);
                left.fill(Qt::transparent);

                {
                    QPainter sidePainter(&left);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    d->background->paint(&sidePainter, QPoint(0, 0), "left");
                }

                p.drawTiledPixmap(QRect(leftOffset, contentTop, d->leftWidth, contentHeight), left);
            }

            if (d->bFlags & DrawRightBorder) {
                QPixmap right(d->rightWidth, leftHeight);
                right.fill(Qt::transparent);

                {
                    QPainter sidePainter(&right);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    d->background->paint(&sidePainter, QPoint(0, 0), "right");
                }

                p.drawTiledPixmap(QRect(rightOffset, contentTop, d->rightWidth, contentHeight), right);
            }

            if (d->bFlags & DrawTopBorder) {
                QPixmap top(topWidth, d->topHeight);
                top.fill(Qt::transparent);

                {
                    QPainter sidePainter(&top);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    d->background->paint(&sidePainter, QPoint(0, 0), "top");
                }

                p.drawTiledPixmap(QRect(contentLeft, topOffset, contentWidth, d->topHeight), top);
            }

            if (d->bFlags & DrawBottomBorder) {
                QPixmap bottom(topWidth, d->bottomHeight);
                bottom.fill(Qt::transparent);

                {
                    QPainter sidePainter(&bottom);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    d->background->paint(&sidePainter, QPoint(0, 0), "bottom");
                }

                p.drawTiledPixmap(QRect(contentLeft, bottomOffset, contentWidth, d->bottomHeight), bottom);
            }
        }

        // re-enable this once Qt's svg rendering is un-buggered
        //resize(contentWidth, contentHeight);
        //paint(&p, QRect(contentLeft, contentTop, contentWidth, contentHeight), "center");
   }

   //p2->drawPixmap(paintRect, *cachedBackground, paintRect.translated(-leftOffset,-topOffset));
   painter->drawPixmap(rect, *d->cachedBackground, rect.translated(-leftOffset, -topOffset));
}

void SvgPanel::updateSizes()
{
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
   d->noBorderPadding = d->background->elementExists("hint-no-border-padding");
   d->stretchBorders = d->background->elementExists("hint-stretch-borders");
   emit repaintNeeded();
}

} // Plasma namespace

#include "svgpanel.moc"
