/*
 *   Copyright 2010 Marco Martin <mart@kde.org>
 *   Copyright 2014 David Edmundson <davidedmundson@kde.org>
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

#include "framesvgitem.h"

#include <QQuickWindow>
#include <QSGTexture>
#include <QDebug>

#include "svgtexturenode.h"

#include <cmath> //floor()

namespace Plasma
{

FrameSvgItemMargins::FrameSvgItemMargins(Plasma::FrameSvg *frameSvg, QObject *parent)
    : QObject(parent),
      m_frameSvg(frameSvg),
      m_fixed(false)
{
    //qDebug() << "margins at: " << left() << top() << right() << bottom();
    connect(m_frameSvg, SIGNAL(repaintNeeded()), this, SLOT(update()));
}

qreal FrameSvgItemMargins::left() const
{
    if (m_fixed) {
        return m_frameSvg->fixedMarginSize(Types::LeftMargin);
    } else {
        return m_frameSvg->marginSize(Types::LeftMargin);
    }
}

qreal FrameSvgItemMargins::top() const
{
    if (m_fixed) {
        return m_frameSvg->fixedMarginSize(Types::TopMargin);
    } else {
        return m_frameSvg->marginSize(Types::TopMargin);
    }
}

qreal FrameSvgItemMargins::right() const
{
    if (m_fixed) {
        return m_frameSvg->fixedMarginSize(Types::RightMargin);
    } else {
        return m_frameSvg->marginSize(Types::RightMargin);
    }
}

qreal FrameSvgItemMargins::bottom() const
{
    if (m_fixed) {
        return m_frameSvg->fixedMarginSize(Types::BottomMargin);
    } else {
        return m_frameSvg->marginSize(Types::BottomMargin);
    }
}

void FrameSvgItemMargins::update()
{
    emit marginsChanged();
}

void FrameSvgItemMargins::setFixed(bool fixed)
{
    if (fixed == m_fixed) {
        return;
    }

    m_fixed = fixed;
    emit marginsChanged();
}

bool FrameSvgItemMargins::isFixed() const
{
    return m_fixed;
}

FrameSvgItem::FrameSvgItem(QQuickItem *parent)
    : QQuickItem(parent),
      m_textureChanged(false)
{
    m_frameSvg = new Plasma::FrameSvg(this);
    m_margins = new FrameSvgItemMargins(m_frameSvg, this);
    m_fixedMargins = new FrameSvgItemMargins(m_frameSvg, this);
    m_fixedMargins->setFixed(true);
    setFlag(ItemHasContents, true);
    connect(m_frameSvg, SIGNAL(repaintNeeded()), this, SLOT(doUpdate()));
    connect(&m_units, &Units::devicePixelRatioChanged, this, &FrameSvgItem::updateDevicePixelRatio);
}

FrameSvgItem::~FrameSvgItem()
{
}

void FrameSvgItem::setImagePath(const QString &path)
{
    if (m_frameSvg->imagePath() == path) {
        return;
    }

    m_frameSvg->setImagePath(path);
    m_frameSvg->setElementPrefix(m_prefix);
    updateDevicePixelRatio();

    if (implicitWidth() <= 0) {
        setImplicitWidth(m_frameSvg->marginSize(Plasma::Types::LeftMargin) + m_frameSvg->marginSize(Plasma::Types::RightMargin));
    }

    if (implicitHeight() <= 0) {
        setImplicitHeight(m_frameSvg->marginSize(Plasma::Types::TopMargin) + m_frameSvg->marginSize(Plasma::Types::BottomMargin));
    }

    emit imagePathChanged();
    m_margins->update();

    if (isComponentComplete()) {
        m_frameSvg->resizeFrame(QSizeF(width(), height()));
        m_textureChanged = true;
        update();
    }
}

QString FrameSvgItem::imagePath() const
{
    return m_frameSvg->imagePath();
}

void FrameSvgItem::setPrefix(const QString &prefix)
{
    if (m_prefix == prefix) {
        return;
    }

    m_frameSvg->setElementPrefix(prefix);
    m_prefix = prefix;

    if (implicitWidth() <= 0) {
        setImplicitWidth(m_frameSvg->marginSize(Plasma::Types::LeftMargin) + m_frameSvg->marginSize(Plasma::Types::RightMargin));
    }

    if (implicitHeight() <= 0) {
        setImplicitHeight(m_frameSvg->marginSize(Plasma::Types::TopMargin) + m_frameSvg->marginSize(Plasma::Types::BottomMargin));
    }

    emit prefixChanged();
    m_margins->update();

    if (isComponentComplete()) {
        m_frameSvg->resizeFrame(QSizeF(width(), height()));
        m_textureChanged = true;
        update();
    }
}

QString FrameSvgItem::prefix() const
{
    return m_prefix;
}

FrameSvgItemMargins *FrameSvgItem::margins() const
{
    return m_margins;
}

FrameSvgItemMargins *FrameSvgItem::fixedMargins() const
{
    return m_fixedMargins;
}

void FrameSvgItem::setEnabledBorders(const Plasma::FrameSvg::EnabledBorders borders)
{
    if (m_frameSvg->enabledBorders() == borders) {
        return;
    }

    m_frameSvg->setEnabledBorders(borders);
    emit enabledBordersChanged();
    m_textureChanged = true;
    update();
}

Plasma::FrameSvg::EnabledBorders FrameSvgItem::enabledBorders() const
{
    return m_frameSvg->enabledBorders();
}

bool FrameSvgItem::hasElementPrefix(const QString &prefix) const
{
    return m_frameSvg->hasElementPrefix(prefix);
}

void FrameSvgItem::geometryChanged(const QRectF &newGeometry,
                                   const QRectF &oldGeometry)
{
    if (isComponentComplete()) {
        m_frameSvg->resizeFrame(newGeometry.size());
        m_textureChanged = true;
    }
    QQuickItem::geometryChanged(newGeometry, oldGeometry);
}

void FrameSvgItem::doUpdate()
{
    if (implicitWidth() <= 0) {
        setImplicitWidth(m_frameSvg->marginSize(Plasma::Types::LeftMargin) + m_frameSvg->marginSize(Plasma::Types::RightMargin));
    }

    if (implicitHeight() <= 0) {
        setImplicitHeight(m_frameSvg->marginSize(Plasma::Types::TopMargin) + m_frameSvg->marginSize(Plasma::Types::BottomMargin));
    }

    m_textureChanged = true;
    update();
}

void FrameSvgItem::setImplicitWidth(qreal width)
{
    if (implicitWidth() == width) {
        return;
    }

    QQuickItem::setImplicitWidth(width);

    emit implicitWidthChanged();
}

qreal FrameSvgItem::implicitWidth() const
{
    return QQuickItem::implicitWidth();
}

void FrameSvgItem::setImplicitHeight(qreal height)
{
    if (implicitHeight() == height) {
        return;
    }

    QQuickItem::setImplicitHeight(height);

    emit implicitHeightChanged();
}

qreal FrameSvgItem::implicitHeight() const
{
    return QQuickItem::implicitHeight();
}

Plasma::FrameSvg *FrameSvgItem::frameSvg() const
{
    return m_frameSvg;
}

SVGTextureNode* FrameSvgItem::createNode(QImage image)
{
    auto node = new SVGTextureNode;
    auto texture = window()->createTextureFromImage(image);
    texture->setFiltering(QSGTexture::Nearest);
    node->setTexture(texture);
    return node;
}

QSGNode *FrameSvgItem::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *)
{
    if (!window() || !m_frameSvg || !m_frameSvg->hasElementPrefix(m_prefix)) {
        delete oldNode;
        return Q_NULLPTR;
    }

    if(!oldNode) { //FIXME or theme changed or enabled borders changed
        delete oldNode;
        oldNode = new QSGNode;
        //we're abusing transformOrigin enum here to get access to values
        //these MUST be uploaded in the same order

        auto topLeft = createNode(m_frameSvg->image(QSize(5,5), "topleft"));
        oldNode->appendChildNode(topLeft);

        auto top = createNode(m_frameSvg->image(QSize(5,5), "top"));
        oldNode->appendChildNode(top);

        auto topRight = createNode(m_frameSvg->image(QSize(5,5), "topright"));
        oldNode->appendChildNode(topRight);

        auto left = createNode(m_frameSvg->image(QSize(5,5), "left"));
        oldNode->appendChildNode(left);

        auto center = createNode(m_frameSvg->image(QSize(5,5), "center"));
        oldNode->appendChildNode(center);

        auto right = createNode(m_frameSvg->image(QSize(5,5), "right"));
        oldNode->appendChildNode(right);

        auto bottomLeft = createNode(m_frameSvg->image(QSize(5,5), "bottomleft"));
        oldNode->appendChildNode(bottomLeft);

        auto bottom = createNode(m_frameSvg->image(QSize(5,5), "bottom"));
        oldNode->appendChildNode(bottom);

        auto bottomRight = createNode(m_frameSvg->image(QSize(5,5), "bottomright"));
        oldNode->appendChildNode(bottomRight);
        //set sizeDirty=true
    }


    //FIXME if (m_sizeDirty)
    {
        //TODO cast root node add a convenience method on it?

        static_cast<SVGTextureNode*>(oldNode->childAtIndex(Top))->setRect(5,0,width()-10, 5);
        static_cast<SVGTextureNode*>(oldNode->childAtIndex(Left))->setRect(0,5, 5,height()-10);
        static_cast<SVGTextureNode*>(oldNode->childAtIndex(Right))->setRect(width()-5,5,5,height()-10);
        static_cast<SVGTextureNode*>(oldNode->childAtIndex(Bottom))->setRect(5,height()-5,width()-10,5);
        static_cast<SVGTextureNode*>(oldNode->childAtIndex(TopLeft))->setRect(0,0,5,5);
        static_cast<SVGTextureNode*>(oldNode->childAtIndex(TopRight))->setRect(width()-5,0,5,5);
        static_cast<SVGTextureNode*>(oldNode->childAtIndex(BottomLeft))->setRect(0,height()-5, 5,5);
        static_cast<SVGTextureNode*>(oldNode->childAtIndex(BottomRight))->setRect(width()-5,height()-5, 5, 5);
        static_cast<SVGTextureNode*>(oldNode->childAtIndex(Center))->setRect(5,5, width()-10, height()-10);


        //TODO sizeDirty = false
    }

//     SVGTextureNode *textureNode = static_cast<SVGTextureNode *>(oldNode);
//     if (!textureNode) {
//         textureNode = new SVGTextureNode;
//         textureNode->setFiltering(QSGTexture::Nearest);
//         m_textureChanged = true; //force updating the texture on our newly created node
//     }
//
//     if (m_textureChanged || textureNode->texture()->textureSize() != m_frameSvg->size()) {
//         const QImage image = m_frameSvg->framePixmap().toImage();
//         QSGTexture *texture = window()->createTextureFromImage(image);
//         texture->setFiltering(QSGTexture::Nearest);
//         textureNode->setTexture(texture);
//         m_textureChanged = false;
//         textureNode->setRect(0, 0, width(), height());
//     }

    return oldNode;
}

void FrameSvgItem::componentComplete()
{
    QQuickItem::componentComplete();
    m_frameSvg->resizeFrame(QSize(width(), height()));
    m_textureChanged = true;
}

void FrameSvgItem::updateDevicePixelRatio()
{
    //devicepixelratio is always set integer in the svg, so needs at least 192dpi to double up.
    //(it needs to be integer to have lines contained inside a svg piece to keep being pixel aligned)
    m_frameSvg->setDevicePixelRatio(qMax<qreal>(1.0, floor(m_units.devicePixelRatio())));
    m_textureChanged = true;
}

} // Plasma namespace

#include "framesvgitem.moc"
