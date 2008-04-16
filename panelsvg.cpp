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

class PanelSvg::Private
{
public:
    Private(PanelSvg *psvg)
      : q(psvg),
        enabledBorders(AllBorders),
        cachedBackground(0),
        contentAtOrigin(false)
    {
    }

    ~Private()
    {
        delete cachedBackground;
    }

    void generateBackground();
    void updateSizes();

    PanelSvg *q;

    EnabledBorders enabledBorders;
    QPixmap *cachedBackground;
    Svg *background;
    QSizeF panelSize;

    Location location;
    QString prefix;

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

PanelSvg::PanelSvg(const QString& imagePath, QObject* parent)
    : Svg(imagePath, parent),
      d(new Private(this))
{
    d->background = new Svg(imagePath, this);
    connect(d->background, SIGNAL(repaintNeeded()), this, SLOT(updateSizes()));

    d->updateSizes();
    d->panelSize = d->background->size();
}

PanelSvg::~PanelSvg()
{
    delete d;
}

void PanelSvg::setImagePath(const QString& imagePath)
{
    if (imagePath == d->background->file()) {
        return;
    }

    d->background->setFile(imagePath);
    setElementPrefix(prefix());
}

QString PanelSvg::imagePath() const
{
   return d->background->file();
}

void PanelSvg::setEnabledBorders(const EnabledBorders borders)
{
    if (borders == d->enabledBorders) {
        return;
    }

    d->enabledBorders = borders;
    d->updateSizes();
}

PanelSvg::EnabledBorders PanelSvg::enabledBorders() const
{
    return d->enabledBorders;
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
    if (!d->background->hasElement(prefix + "-center")) {
        d->prefix.clear();
    } else {
        d->prefix = prefix;
        if (!d->prefix.isEmpty()) {
            d->prefix += '-';
        }
    }

    d->location = Floating;

    if (d->cachedBackground) {
        d->updateSizes();
    }
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
    if (!size.isValid() || size.width() < 1 || size.height() < 1 || size == d->panelSize) {
        return;
    }

    d->panelSize = size;
    d->updateSizes();
}

void PanelSvg::resize(qreal width, qreal height)
{
    resize(QSize(width, height));
}

qreal PanelSvg::marginSize(const Plasma::MarginEdge edge) const
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

QBitmap PanelSvg::mask() const
{
    if (!d->cachedBackground) {
        d->generateBackground();
    }

    return d->cachedBackground->alphaChannel().createMaskFromColor(Qt::black);
}

void PanelSvg::paint(QPainter* painter, const QRectF& rect, const QPointF& pos)
{
    if (!d->cachedBackground) {
        d->generateBackground();
    }

    //FIXME: this is redundant with generatebackground for now
    bool origined = d->contentAtOrigin;
    const int topOffset = origined ? 0 - d->topHeight : 0;
    const int leftOffset = origined ? 0 - d->leftWidth : 0;

    painter->drawPixmap(rect, *d->cachedBackground, rect.translated(-pos.x()-leftOffset,-pos.y()-topOffset));
}

void PanelSvg::Private::generateBackground()
{
    bool origined = contentAtOrigin;
    const int topWidth = background->elementSize(prefix + "top").width();
    const int leftHeight = background->elementSize(prefix + "left").height();
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
               centerTileHeight = background->elementSize(prefix + "center").height();
               centerTileWidth = background->elementSize(prefix + "center").width();
               QPixmap center(centerTileWidth, centerTileHeight);
               center.fill(Qt::transparent);

               {
                   QPainter centerPainter(&center);
                   centerPainter.setCompositionMode(QPainter::CompositionMode_Source);
                   background->paint(&centerPainter, QPoint(0, 0), prefix + "center");
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
                                   prefix + "center");
               background->resize();
           }
       }


        // Corners
        if (enabledBorders & TopBorder) {
            if (!origined) {
                contentTop = topHeight;
                bottomOffset += topHeight;
            }

            if (enabledBorders & LeftBorder) {
                background->paint(&p, QRect(leftOffset, topOffset, leftWidth, topHeight), prefix + "topleft");

                if (!origined) {
                    contentLeft = leftWidth;
                    rightOffset = contentWidth + leftWidth;
                }
            }

            if (enabledBorders & RightBorder) {
                background->paint(&p, QRect(rightOffset, topOffset, rightWidth, topHeight), prefix + "topright");
            }
        }

        if (enabledBorders & BottomBorder) {
            if (enabledBorders & LeftBorder) {
                background->paint(&p, QRect(leftOffset, bottomOffset, leftWidth, bottomHeight), prefix + "bottomleft");

                if (!origined) {
                    contentLeft = leftWidth;
                    rightOffset = contentWidth + leftWidth;
                }
            }

            if (enabledBorders & RightBorder) {
                background->paint(&p, QRect(rightOffset, bottomOffset, rightWidth, bottomHeight), prefix + "bottomright");
            }
        }

        // Sides
        if (stretchBorders) {
            if (enabledBorders & LeftBorder) {
                background->paint(&p, QRect(leftOffset, contentTop, leftWidth, contentHeight), prefix + "left");
            }

            if (enabledBorders & RightBorder) {
                background->paint(&p, QRect(rightOffset, contentTop, rightWidth, contentHeight), prefix + "right");
            }

            if (enabledBorders & TopBorder) {
                background->paint(&p, QRect(contentLeft, topOffset, contentWidth, topHeight), prefix + "top");
            }

            if (enabledBorders & BottomBorder) {
                background->paint(&p, QRect(contentLeft, bottomOffset, contentWidth, bottomHeight), prefix + "bottom");
            }
        } else {
            if (enabledBorders & LeftBorder) {
                QPixmap left(leftWidth, leftHeight);
                left.fill(Qt::transparent);

                {
                    QPainter sidePainter(&left);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    background->paint(&sidePainter, QPoint(0, 0), prefix + "left");
                }

                p.drawTiledPixmap(QRect(leftOffset, contentTop, leftWidth, contentHeight), left);
            }

            if (enabledBorders & RightBorder) {
                QPixmap right(rightWidth, leftHeight);
                right.fill(Qt::transparent);

                {
                    QPainter sidePainter(&right);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    background->paint(&sidePainter, QPoint(0, 0), prefix + "right");
                }

                p.drawTiledPixmap(QRect(rightOffset, contentTop, rightWidth, contentHeight), right);
            }

            if (enabledBorders & TopBorder) {
                QPixmap top(topWidth, topHeight);
                top.fill(Qt::transparent);

                {
                    QPainter sidePainter(&top);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    background->paint(&sidePainter, QPoint(0, 0), prefix + "top");
                }

                p.drawTiledPixmap(QRect(contentLeft, topOffset, contentWidth, topHeight), top);
            }

            if (enabledBorders & BottomBorder) {
                QPixmap bottom(topWidth, bottomHeight);
                bottom.fill(Qt::transparent);

                {
                    QPainter sidePainter(&bottom);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    background->paint(&sidePainter, QPoint(0, 0), prefix + "bottom");
                }

                p.drawTiledPixmap(QRect(contentLeft, bottomOffset, contentWidth, bottomHeight), bottom);
            }
        }

        // re-enable this once Qt's svg rendering is un-buggered
        //resize(contentWidth, contentHeight);
        //paint(&p, QRect(contentLeft, contentTop, contentWidth, contentHeight), "center");
   }
}

void PanelSvg::Private::updateSizes()
{
    delete cachedBackground;
    cachedBackground = 0;

    background->resize();
    if (enabledBorders & TopBorder) {
        topHeight = background->elementSize(prefix + "top").height();
    } else {
        topHeight = 0;
    }

    if (enabledBorders & LeftBorder) {
        leftWidth = background->elementSize(prefix + "left").width();
    } else {
        leftWidth = 0;
    }

    if (enabledBorders & RightBorder) {
        rightWidth = background->elementSize(prefix + "right").width();
    } else {
        rightWidth = 0;
    }

   if (enabledBorders & BottomBorder) {
       bottomHeight = background->elementSize(prefix + "bottom").height();
   } else {
       bottomHeight = 0;
   }

   //since it's rectangular, topWidth and bottomWidth must be the same
   tileCenter = background->hasElement("hint-tile-center");
   noBorderPadding = background->hasElement("hint-no-border-padding");
   stretchBorders = background->hasElement("hint-stretch-borders");
   emit q->repaintNeeded();
}

} // Plasma namespace

#include "panelsvg.moc"
