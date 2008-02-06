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

SvgPanel::SvgPanel(const QString& imagePath, QObject* parent)
    : QObject(parent),
      bFlags(DrawTop|DrawBottom|DrawLeft|DrawRight|ContentAtOrigin),
      cachedBackground(0)
{
    m_background = new Svg(imagePath, this);
    connect(m_background, SIGNAL(repaintNeeded()), this, SLOT(updateSizes()));

    updateSizes();
    panelSize = m_background->size();
}

SvgPanel::~SvgPanel()
{
}

void SvgPanel::setFile(const QString& imagePath)
{
    if (imagePath == m_background->file()) {
        return;
    }

    delete cachedBackground;
    cachedBackground = 0;
    m_background->setFile(imagePath);
    updateSizes();
}

QString SvgPanel::file() const
{
   return m_background->file();
}

void SvgPanel::setBorderFlags(const BorderFlags flags)
{
    if (flags != bFlags) {
        delete cachedBackground;
        cachedBackground = 0;
    }

    bFlags = flags;
    updateSizes();
}

SvgPanel::BorderFlags SvgPanel::borderFlags() const
{
    return bFlags;
}

void SvgPanel::resize(const QSizeF& size)
{
    if (!size.isValid() || size.width() < 1 || size.height() < 1 || size == panelSize) {
        return;
    }

    delete cachedBackground;
    cachedBackground = 0;
    updateSizes();
    panelSize = size;
}

qreal SvgPanel::marginSize(const Plasma::Layout::MarginEdge edge) const
{
    if (noBorderPadding) {
        return .0;
    }

    switch (edge) {
    case Plasma::Layout::TopMargin:
        return topHeight;
    break;

    case Plasma::Layout::LeftMargin:
        return leftWidth;
    break;

    case Plasma::Layout::RightMargin:
        return rightWidth;
    break;

    //Plasma::Layout::BottomMargin
    default:
        return bottomHeight;
    break;
    }
}

void SvgPanel::paint(QPainter* painter, const QRectF& rect)
{
    bool useOrigin = bFlags & ContentAtOrigin;
    const int topOffset = useOrigin ? 0 - topHeight : 0;
    const int leftOffset = useOrigin ? 0 - leftWidth : 0;

    if (!cachedBackground) {
        const int contentTop = 0;
        const int contentLeft = 0;
        const int contentWidth = panelSize.width() - leftWidth  - rightWidth;
        const int contentHeight = panelSize.height() - topHeight  - bottomHeight;
        const int rightOffset = contentWidth;
        const int bottomOffset = contentHeight;

        scaledSize = QSizeF(panelSize.width() -
                            (leftWidth+rightWidth) +
                            panelSize.width()*(((qreal)(leftWidth+rightWidth))/panelSize.width()),
                            panelSize.height() -
                            (topHeight+bottomHeight) +
                            panelSize.height()*(((qreal)(topHeight+bottomHeight))/panelSize.height()));

        delete cachedBackground;
        cachedBackground = new QPixmap(leftWidth + contentWidth + rightWidth, topHeight + contentHeight + bottomHeight);
        cachedBackground->fill(Qt::transparent);
        QPainter p(cachedBackground);
        p.translate(leftWidth, topHeight);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.setRenderHint(QPainter::SmoothPixmapTransform);

        //FIXME: This is a hack to fix a drawing problems with svg files where a thin transparent border is drawn around the svg image.
        //       the transparent border around the svg seems to vary in size depending on the size of the svg and as a result increasing the
        //       svg image by 2 all around didn't resolve the issue. For now it resizes based on the border size.


       //CENTER
       if (contentHeight > 0 && contentWidth > 0) {
           m_background->resize(scaledSize.width(), scaledSize.height());
           m_background->paint(&p, QRect(contentLeft-leftWidth, contentTop-topHeight,
                                         contentWidth+leftWidth*2, contentHeight+topHeight*2),
                               "center");
           m_background->resize();
       }

        //EDGES
        if (bFlags & DrawTop) {
            if (bFlags & DrawLeft) {
                m_background->paint(&p, QRect(leftOffset, topOffset, leftWidth, topHeight), "topleft");
            }

            if (bFlags & DrawRight) {
                m_background->paint(&p, QRect(rightOffset, topOffset,rightWidth, topHeight), "topright");
            }
        }

        if (bFlags & DrawBottom) {
            if (bFlags & DrawLeft) {
                m_background->paint(&p, QRect(leftOffset, bottomOffset, leftWidth, bottomHeight), "bottomleft");
            }

            if (bFlags & DrawRight) {
                m_background->paint(&p, QRect(rightOffset, bottomOffset, rightWidth, bottomHeight), "bottomright");
            }
        }

        //SIDES
        if (stretchBorders) {
            if (bFlags & DrawLeft) {
                m_background->resize(m_background->size().width(), scaledSize.height());
                m_background->paint(&p, QRect(leftOffset, contentTop, leftWidth, contentHeight), "left");
                m_background->resize();
            }

            if (bFlags & DrawRight) {
                m_background->resize(m_background->size().width(), scaledSize.height());
                m_background->paint(&p, QRect(rightOffset, contentTop, rightWidth, contentHeight), "right");
                m_background->resize();
            }

            if (bFlags & DrawTop) {
                m_background->resize(scaledSize.width(), m_background->size().height());
                m_background->paint(&p, QRect(contentLeft, topOffset, contentWidth, topHeight), "top");
                m_background->resize();
            }

            if (bFlags & DrawBottom) {
                m_background->resize(scaledSize.width(), m_background->size().height());
                m_background->paint(&p, QRect(contentLeft, bottomOffset, contentWidth, bottomHeight), "bottom");
                m_background->resize();
            }
        } else {
            if (bFlags & DrawLeft) {
                QPixmap left(leftWidth, leftHeight);
                left.fill(Qt::transparent);

                {
                    QPainter sidePainter(&left);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    m_background->paint(&sidePainter, QPoint(0, 0), "left");
                }

                p.drawTiledPixmap(QRect(leftOffset, contentTop, leftWidth, contentHeight), left);
            }

            if (bFlags & DrawRight) {
                QPixmap right(rightWidth, leftHeight);
                right.fill(Qt::transparent);

                {
                    QPainter sidePainter(&right);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    m_background->paint(&sidePainter, QPoint(0, 0), "right");
                }

                p.drawTiledPixmap(QRect(rightOffset, contentTop, rightWidth, contentHeight), right);
            }

            if (bFlags & DrawTop) {
                QPixmap top(topWidth, topHeight);
                top.fill(Qt::transparent);

                {
                    QPainter sidePainter(&top);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    m_background->paint(&sidePainter, QPoint(0, 0), "top");
                }

                p.drawTiledPixmap(QRect(contentLeft, topOffset, contentWidth, topHeight), top);
            }

            if (bFlags & DrawBottom) {
                QPixmap bottom(topWidth, bottomHeight);
                bottom.fill(Qt::transparent);

                {
                    QPainter sidePainter(&bottom);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    m_background->paint(&sidePainter, QPoint(0, 0), "bottom");
                }

                p.drawTiledPixmap(QRect(contentLeft, bottomOffset, contentWidth, bottomHeight), bottom);
            }
        }

        // re-enable this once Qt's svg rendering is un-buggered
        //resize(contentWidth, contentHeight);
        //paint(&p, QRect(contentLeft, contentTop, contentWidth, contentHeight), "center");
   }

   //p2->drawPixmap(paintRect, *cachedBackground, paintRect.translated(-leftOffset,-topOffset));
   painter->drawPixmap(rect, *cachedBackground, rect.translated(-leftOffset, -topOffset));
}

void SvgPanel::updateSizes()
{
    m_background->resize();
    if (bFlags & DrawTop) {
        topHeight = m_background->elementSize("top").height();
    } else {
        topHeight = 0;
    }

    if (bFlags & DrawLeft) {
        leftWidth = m_background->elementSize("left").width();
    } else {
        leftWidth = 0;
    }

    if (bFlags & DrawRight) {
        rightWidth = m_background->elementSize("right").width();
    } else {
        rightWidth = 0;
    }

   if (bFlags & DrawBottom) {
       bottomHeight = m_background->elementSize("bottom").height();
   } else {
       bottomHeight = 0;
   }

   //since it's rectangular, topWidth and bottomWidth must be the same
   topWidth = m_background->elementSize("top").width();
   leftHeight = m_background->elementSize("left").height();
   noBorderPadding = m_background->elementExists("hint-no-border-padding");
   stretchBorders = m_background->elementExists("hint-stretch-borders");
}


} // Plasma namespace

#include "svgpanel.moc"
